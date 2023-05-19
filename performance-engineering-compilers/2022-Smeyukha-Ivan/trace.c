#include <stdio.h>
#include <stddef.h>
#include "dr_api.h"
#include "drmgr.h"
#include "drreg.h"
#include "drutil.h"
#include "drx.h"
#include "drwrap.h"
#include "drsyms.h"

/* Max number of mem_ref a buffer can have. It should be big enough
 * to hold all entries between clean calls.
 */
#define MAX_NUM_MEM_REFS 4096
/* The maximum size of buffer for holding mem_refs. */
#define MEM_BUF_SIZE (sizeof(mem_ref_t) * MAX_NUM_MEM_REFS)

enum {
    REF_TYPE_READ = 0,
    REF_TYPE_WRITE = 1,
};

/* representing a memory reference instruction or the reference information */
typedef struct _mem_ref_t {
    ushort type; /* r(0), w(1), or opcode (assuming 0/1 are invalid opcode) */
    ushort size; /* mem ref size or instr length */
    app_pc addr; /* mem ref addr or instr pc */
} mem_ref_t;

/* thread private log file and counter */
typedef struct _per_thread_t{
    byte *seg_base;
    mem_ref_t *buf_base;
    file_t log;
    FILE *logf;
    uint64 num_refs;
} per_thread_t;

typedef struct _addr_line_t{
    app_pc addr;
    uint64 line;
    char name[100];
    size_t size;
} addr_line_t;
addr_line_t *struct_malloc;
static uint64 struct_size;

file_t file;
FILE* f;

static app_pc upper_pc;
static void *mutex;        /* for multithread support */
static uint64 num_refs;    /* keep a global memory reference count */

/* Allocated TLS slot offsets */
enum {
    MEMTRACE_TLS_OFFS_BUF_PTR,
    MEMTRACE_TLS_COUNT, /* total number of TLS slots allocated */
};
static reg_id_t tls_seg;
static uint tls_offs;
static int tls_idx;
#define TLS_SLOT(tls_base, enum_val) (void **)((byte *)(tls_base) + tls_offs + (enum_val))
#define BUF_PTR(tls_base) *(mem_ref_t **)TLS_SLOT(tls_base, MEMTRACE_TLS_OFFS_BUF_PTR)

/* open a log file */
file_t log_file_open() {
    return dr_open_file("tracefile.txt", DR_FILE_CLOSE_ON_FORK | DR_FILE_ALLOW_LARGE | DR_FILE_WRITE_OVERWRITE);
}

/* close a log file opened by log_file_open */
void log_file_close(file_t file) {
    dr_close_file(file);
}

/* Converts a raw file descriptor into a FILE stream. */
FILE *log_stream_from_file(file_t file) {
    return fdopen(file, "w");
}

/* log_file_close does *not* need to be called when calling this on a stream converted from a file descriptor. */
void log_stream_close(FILE *file) {
    fclose(file);
}

static void memtrace(void *drcontext) {
    per_thread_t *data;
    mem_ref_t *mem_ref, *buf_ptr;

    data = drmgr_get_tls_field(drcontext, tls_idx);
    buf_ptr = BUF_PTR(data->seg_base);

    for (mem_ref = (mem_ref_t *)data->buf_base; mem_ref < buf_ptr; mem_ref++) {

        if (mem_ref->type <= REF_TYPE_WRITE && (!upper_pc || mem_ref->addr <= upper_pc)) {
            ptr_uint_t addr = (ptr_uint_t)mem_ref->addr;
            for (int i = 0; i < struct_size; ++i) {
                app_pc a = struct_malloc[i].addr;
                size_t s = struct_malloc[i].size;
                if(addr >= a && addr <= a + s){
                    fprintf(data->logf, "%ld %d %s %s\n",
                            addr,
                            mem_ref->size,
                            mem_ref->type == 1 ? "w" : "r",
                            struct_malloc[i].name
                            );
                    break;
                }
            }
        }

        data->num_refs++;
    }
    BUF_PTR(data->seg_base) = data->buf_base;
}

/* clean_call dumps the memory reference info to the log file */
static void clean_call(void) {
    memtrace(dr_get_current_drcontext());
}

uint64 time_start;

//Пре-функция обертки целевой функции
static void
wrap_pre_malloc(void *wrapcxt, OUT void **user_data)
{
    /*Получаем первый аргумент malloc,
      который хранит в себе количество выделенной памяти
      и добавляем его в разделяемую между оборачивающими
      функциями область памяти*/
    size_t sz = (size_t)drwrap_get_arg(wrapcxt, 0);
    *user_data = (void *)sz;
    time_start = dr_get_microseconds();
}

//Пре-функция обертки целевой функции
static void
wrap_pre_realloc(void *wrapcxt, OUT void **user_data)
{
    /*Получаем первый аргумент malloc,
      который хранит в себе количество выделенной памяти
      и добавляем его в разделяемую между оборачивающими
      функциями область памяти*/
    size_t sz = (size_t)drwrap_get_arg(wrapcxt, 1);
    *user_data = (void *)sz;
}

//Пре-функция обертки целевой функции
static void
wrap_pre_calloc(void *wrapcxt, OUT void **user_data)
{
    /*Получаем первый аргумент malloc,
      который хранит в себе количество выделенной памяти
      и добавляем его в разделяемую между оборачивающими
      функциями область памяти*/
    size_t sz = (size_t)drwrap_get_arg(wrapcxt, 0) * (size_t)drwrap_get_arg(wrapcxt, 1);
    *user_data = (void *)sz;
}

//Пост-функция обертки целевой функции
static void
wrap_post(void *wrapcxt, void *user_data)
{
    uint64 time_end = dr_get_microseconds();
    module_data_t *mod;
    //Получаем адрес, где вызывается malloc и откуда он вызывается
    app_pc ret_addr = (app_pc)drwrap_get_retaddr(wrapcxt);
    app_pc ret_val = drwrap_get_retval(wrapcxt);
    //Получаем указатель на модуль по адресу
    mod = dr_lookup_module(ret_addr);
    //Объявляем структуру, хранящую информацию о символах
    //и заполняем ее базовыми значениями
    drsym_error_t symres;
    drsym_info_t sym;
    char name[256];
    char file[256];
    sym.struct_size = sizeof(sym);
    sym.name = name;
    sym.name_size = 256;
    sym.file = file;
    sym.file_size = 256;
    //Получаем структуру с информацией о символах для malloc
    symres = drsym_lookup_address(mod->full_path, ret_addr - mod->start,
                                  &sym, DRSYM_DEFAULT_FLAGS);
    //Выводим всю необходимую информацию в лог-файл
    if(sym.line > 0 ){
        fprintf(f, "%s %s %d %ld %d %ld %d\n", sym.file, sym.name,
                sym.line, ret_val, (size_t)user_data, ret_addr, time_end - time_start);
        for (int i = 0; i < struct_size; ++i) {
            if(struct_malloc[i].line == sym.line){
                struct_malloc[i].addr = ret_val;
                struct_malloc[i].size = (size_t)user_data;
            }
        }
    }
    dr_free_module_data(mod);
}


//Функция обратного вызова для события загрузки модуля
static void
module_load_event(void *drcontext, const module_data_t *mod, bool loaded)
{
    app_pc towrap = (app_pc)dr_get_proc_address(mod->handle, "malloc");
    if(towrap != NULL){
        drwrap_wrap(towrap, wrap_pre_malloc, wrap_post);
    }
}

static void insert_load_buf_ptr(void *drcontext, instrlist_t *ilist, instr_t *where, reg_id_t reg_ptr) {
    dr_insert_read_raw_tls(drcontext, ilist, where, tls_seg,tls_offs + MEMTRACE_TLS_OFFS_BUF_PTR, reg_ptr);
}

static void insert_update_buf_ptr(void *drcontext, instrlist_t *ilist, instr_t *where, reg_id_t reg_ptr, int adjust) {
    instrlist_meta_preinsert(ilist, where,XINST_CREATE_add(drcontext, opnd_create_reg(reg_ptr), OPND_CREATE_INT16(adjust)));
    dr_insert_write_raw_tls(drcontext, ilist, where, tls_seg,tls_offs + MEMTRACE_TLS_OFFS_BUF_PTR, reg_ptr);
}

static void insert_save_type(void *drcontext, instrlist_t *ilist, instr_t *where, reg_id_t base, reg_id_t scratch, ushort type) {
    scratch = reg_resize_to_opsz(scratch, OPSZ_2);
    instrlist_meta_preinsert(ilist, where,XINST_CREATE_load_int(drcontext, opnd_create_reg(scratch), OPND_CREATE_INT16(type)));
    instrlist_meta_preinsert(ilist, where,XINST_CREATE_store_2bytes(drcontext, OPND_CREATE_MEM16(base, offsetof(mem_ref_t, type)), opnd_create_reg(scratch)));
}

static void insert_save_size(void *drcontext, instrlist_t *ilist, instr_t *where, reg_id_t base, reg_id_t scratch, ushort size) {
    scratch = reg_resize_to_opsz(scratch, OPSZ_2);
    instrlist_meta_preinsert(ilist, where,XINST_CREATE_load_int(drcontext, opnd_create_reg(scratch), OPND_CREATE_INT16(size)));
    instrlist_meta_preinsert(ilist, where,XINST_CREATE_store_2bytes(drcontext, OPND_CREATE_MEM16(base, offsetof(mem_ref_t, size)), opnd_create_reg(scratch)));
}

static void insert_save_pc(void *drcontext, instrlist_t *ilist, instr_t *where, reg_id_t base, reg_id_t scratch, app_pc pc) {
    instrlist_insert_mov_immed_ptrsz(drcontext, (ptr_int_t)pc, opnd_create_reg(scratch), ilist, where, NULL, NULL);
    instrlist_meta_preinsert(ilist, where,XINST_CREATE_store(drcontext, OPND_CREATE_MEMPTR(base, offsetof(mem_ref_t, addr)), opnd_create_reg(scratch)));
}

static void insert_save_addr(void *drcontext, instrlist_t *ilist, instr_t *where, opnd_t ref, reg_id_t reg_ptr, reg_id_t reg_addr) {
    DR_ASSERT(drutil_insert_get_mem_addr(drcontext, ilist, where, ref, reg_addr, reg_ptr));
    insert_load_buf_ptr(drcontext, ilist, where, reg_ptr);
    instrlist_meta_preinsert(ilist, where,XINST_CREATE_store(drcontext, OPND_CREATE_MEMPTR(reg_ptr, offsetof(mem_ref_t, addr)), opnd_create_reg(reg_addr)));
}

/* insert inline code to add an instruction entry into the buffer */
static void instrument_instr(void *drcontext, instrlist_t *ilist, instr_t *where, instr_t *instr) {
    /* We need two scratch registers */
    reg_id_t reg_ptr, reg_tmp;
    /* we don't want to predicate this, because an instruction fetch always occurs */
    instrlist_set_auto_predicate(ilist, DR_PRED_NONE);
    if (drreg_reserve_register(drcontext, ilist, where, NULL, &reg_ptr) != DRREG_SUCCESS ||
        drreg_reserve_register(drcontext, ilist, where, NULL, &reg_tmp) != DRREG_SUCCESS) {
        DR_ASSERT(false); /* cannot recover */
        return;
    }
    insert_load_buf_ptr(drcontext, ilist, where, reg_ptr);
    insert_save_type(drcontext, ilist, where, reg_ptr, reg_tmp,(ushort)instr_get_opcode(instr));
    insert_save_size(drcontext, ilist, where, reg_ptr, reg_tmp,(ushort)instr_length(drcontext, instr));
    insert_save_pc(drcontext, ilist, where, reg_ptr, reg_tmp, instr_get_app_pc(instr));
    insert_update_buf_ptr(drcontext, ilist, where, reg_ptr, sizeof(mem_ref_t));
    /* Restore scratch registers */
    if (drreg_unreserve_register(drcontext, ilist, where, reg_ptr) != DRREG_SUCCESS ||
        drreg_unreserve_register(drcontext, ilist, where, reg_tmp) != DRREG_SUCCESS) {
        DR_ASSERT(false);
    }

    instrlist_set_auto_predicate(ilist, instr_get_predicate(where));
}

/* insert inline code to add a memory reference info entry into the buffer */
static void instrument_mem(void *drcontext, instrlist_t *ilist, instr_t *where, opnd_t ref, bool write) {
    /* We need two scratch registers */
    reg_id_t reg_ptr, reg_tmp;
    if (drreg_reserve_register(drcontext, ilist, where, NULL, &reg_ptr) != DRREG_SUCCESS ||
        drreg_reserve_register(drcontext, ilist, where, NULL, &reg_tmp) != DRREG_SUCCESS) {
        DR_ASSERT(false); /* cannot recover */
        return;
    }
    /* save_addr should be called first as reg_ptr or reg_tmp maybe used in ref */
    insert_save_addr(drcontext, ilist, where, ref, reg_ptr, reg_tmp);
    insert_save_type(drcontext, ilist, where, reg_ptr, reg_tmp,write ? REF_TYPE_WRITE : REF_TYPE_READ);
    insert_save_size(drcontext, ilist, where, reg_ptr, reg_tmp,(ushort)drutil_opnd_mem_size_in_bytes(ref, where));
    insert_update_buf_ptr(drcontext, ilist, where, reg_ptr, sizeof(mem_ref_t));
    /* Restore scratch registers */
    if (drreg_unreserve_register(drcontext, ilist, where, reg_ptr) != DRREG_SUCCESS ||
        drreg_unreserve_register(drcontext, ilist, where, reg_tmp) != DRREG_SUCCESS) {
        DR_ASSERT(false);
    }
}

/* For each memory reference app instr, we insert inline code to fill the buffer
 * with an instruction entry and memory reference entries.
 */
static dr_emit_flags_t event_app_instruction(void *drcontext, void *tag, instrlist_t *bb, instr_t *where, bool for_trace, bool translating, void *user_data) {
    /* Insert code to add an entry for each app instruction. */
    /* Use the drmgr_orig_app_instr_* interface to properly handle our own use
     * of drutil_expand_rep_string() and drx_expand_scatter_gather() (as well
     * as another client/library emulating the instruction stream).
     */
    instr_t *instr_fetch = drmgr_orig_app_instr_for_fetch(drcontext);
    if (instr_fetch != NULL && (instr_reads_memory(instr_fetch) || instr_writes_memory(instr_fetch))) {
        DR_ASSERT(instr_is_app(instr_fetch));
        instrument_instr(drcontext, bb, where, instr_fetch);
    }

    /* Insert code to add an entry for each memory reference opnd. */
    instr_t *instr_operands = drmgr_orig_app_instr_for_operands(drcontext);
    if (instr_operands == NULL || (!instr_reads_memory(instr_operands) && !instr_writes_memory(instr_operands))) {
        return DR_EMIT_DEFAULT;
    }

    DR_ASSERT(instr_is_app(instr_operands));

    for (int i = 0; i < instr_num_srcs(instr_operands); i++) {
        if (opnd_is_memory_reference(instr_get_src(instr_operands,i))) {
            instrument_mem(drcontext,bb, where,instr_get_src(instr_operands,i),false);
        }
    }

    for (int i = 0; i < instr_num_dsts(instr_operands); i++) {
        if (opnd_is_memory_reference(instr_get_dst(instr_operands,i))) {
            instrument_mem(drcontext,bb, where,instr_get_dst(instr_operands,i),true);
        }
    }

    /* insert code to call clean_call for processing the buffer */
    if (/* XXX i#1698: there are constraints for code between ldrex/strex pairs,
         * so we minimize the instrumentation in between by skipping the clean call.
         * As we're only inserting instrumentation on a memory reference, and the
         * app should be avoiding memory accesses in between the ldrex...strex,
         * the only problematic point should be before the strex.
         * However, there is still a chance that the instrumentation code may clear the
         * exclusive monitor state.
         * Using a fault to handle a full buffer should be more robust, and the
         * forthcoming buffer filling API (i#513) will provide that.
         */
            IF_AARCHXX_ELSE(!instr_is_exclusive_store(instr_operands), true))
        dr_insert_clean_call(drcontext, bb, where, (void *)clean_call, false, 0);

    return DR_EMIT_DEFAULT;
}

/* We transform string loops into regular loops so we can more easily
 * monitor every memory reference they make.
 */
static dr_emit_flags_t event_bb_app2app(void *drcontext, void *tag, instrlist_t *bb, bool for_trace, bool translating) {
    if (!drutil_expand_rep_string(drcontext, bb)) {
        DR_ASSERT(false);
        /* in release build, carry on: we'll just miss per-iter refs */
    }
    if (!drx_expand_scatter_gather(drcontext, bb, NULL)) {
        DR_ASSERT(false);
    }
    return DR_EMIT_DEFAULT;
}

static void event_thread_init(void *drcontext) {
    file = dr_open_file("trace.txt", DR_FILE_CLOSE_ON_FORK | DR_FILE_ALLOW_LARGE |
                                     DR_FILE_WRITE_OVERWRITE);
    f = log_stream_from_file(file);

    file_t m_file = dr_open_file("../test/mallocfile.txt", DR_FILE_CLOSE_ON_FORK | DR_FILE_ALLOW_LARGE |
                                            DR_FILE_READ);
    FILE* m_f = fdopen(m_file, "r");

    struct_size = 0;
    struct_malloc = malloc(sizeof(addr_line_t));

    int i = 0;
    char str[100];
    bool f = true;
    int line;
    while (fscanf(m_f, "%s", str) != EOF) {
        char name[100];
        if(f == true){
            struct_size++;
            struct_malloc = realloc(struct_malloc, sizeof(addr_line_t) * struct_size);
            sscanf(str, "%d", &line);
            struct_malloc[struct_size - 1].line = line;
            f = false;
        }
        else{
            strcpy(struct_malloc[struct_size - 1].name, str);
            f = true;
        }
        i++;
    }

    log_stream_close(m_f);
    per_thread_t *data = dr_thread_alloc(drcontext, sizeof(per_thread_t));
    DR_ASSERT(data != NULL);
    drmgr_set_tls_field(drcontext, tls_idx, data);

    /* Keep seg_base in a per-thread data structure so we can get the TLS
     * slot and find where the pointer points to in the buffer.
     */
    data->seg_base = dr_get_dr_segment_base(tls_seg);
    data->buf_base = dr_raw_mem_alloc(MEM_BUF_SIZE, DR_MEMPROT_READ | DR_MEMPROT_WRITE, NULL);
    DR_ASSERT(data->seg_base != NULL && data->buf_base != NULL);
    /* put buf_base to TLS as starting buf_ptr */
    BUF_PTR(data->seg_base) = data->buf_base;

    data->num_refs = 0;

    data->log = log_file_open();
    data->logf = log_stream_from_file(data->log);

}

static void event_thread_exit(void *drcontext) {
    /* dump any remaining buffer entries */
    memtrace(drcontext);
    log_stream_close(f);

    per_thread_t *data = drmgr_get_tls_field(drcontext, tls_idx);
    dr_mutex_lock(mutex);
    num_refs += data->num_refs;
    dr_mutex_unlock(mutex);

    log_stream_close(data->logf);

    dr_raw_mem_free(data->buf_base, MEM_BUF_SIZE);
    dr_thread_free(drcontext, data, sizeof(per_thread_t));
}

static void event_exit(void) {
    dr_log(NULL, DR_LOG_ALL, 1, "Client 'memtrace' num refs seen: " SZFMT "\n", num_refs);
    if (!dr_raw_tls_cfree(tls_offs, MEMTRACE_TLS_COUNT)) {
        DR_ASSERT(false);
    }

    if (!drmgr_unregister_tls_field(tls_idx) ||
        !drmgr_unregister_thread_init_event(event_thread_init) ||
        !drmgr_unregister_thread_exit_event(event_thread_exit) ||
        !drmgr_unregister_bb_app2app_event(event_bb_app2app) ||
            !drmgr_unregister_module_load_event(module_load_event) ||
        !drmgr_unregister_bb_insertion_event(event_app_instruction) ||
        drreg_exit() != DRREG_SUCCESS) {
        DR_ASSERT(false);
    }

    drsym_exit();
    drwrap_exit();

    dr_mutex_destroy(mutex);
    drutil_exit();
    drmgr_exit();
    drx_exit();
}

DR_EXPORT void dr_client_main(client_id_t id, int argc, const char *argv[]) {

    /* We need 2 reg slots beyond drreg's eflags slots => 3 slots */
    drreg_options_t ops = { sizeof(ops), 3, false };
    dr_set_client_name("DynamoRIO Sample Client 'memtrace'","http://dynamorio.org/issues");

    if (argc > 1) {
        upper_pc = (app_pc)strtol(argv[1], NULL, 16);
    } else {
        upper_pc = 0;
    }

    if (!drmgr_init() || drreg_init(&ops) != DRREG_SUCCESS || !drutil_init() || !drx_init()) {
        DR_ASSERT(false);
    }
    drsym_init(0);
    drwrap_init();

    /* register events */
    dr_register_exit_event(event_exit);
    if (!drmgr_register_thread_init_event(event_thread_init) ||
        !drmgr_register_thread_exit_event(event_thread_exit) ||
        !drmgr_register_bb_app2app_event(event_bb_app2app, NULL) ||
            !drmgr_register_module_load_event(module_load_event) ||
        !drmgr_register_bb_instrumentation_event(NULL,event_app_instruction, NULL)) {
        DR_ASSERT(false);
    }

    mutex = dr_mutex_create();

    tls_idx = drmgr_register_tls_field();
    DR_ASSERT(tls_idx != -1);
    /* The TLS field provided by DR cannot be directly accessed from the code cache.
     * For better performance, we allocate raw TLS so that we can directly
     * access and update it with a single instruction.
     */
    if (!dr_raw_tls_calloc(&tls_seg, &tls_offs, MEMTRACE_TLS_COUNT, 0)) {
        DR_ASSERT(false);
    }

    /* make it easy to tell, by looking at log file, which client executed */
    dr_log(NULL, DR_LOG_ALL, 1, "Client 'memtrace' initializing\n");
}
