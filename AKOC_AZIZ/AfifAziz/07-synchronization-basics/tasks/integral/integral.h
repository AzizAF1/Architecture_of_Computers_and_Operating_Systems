#include <stdint.h>
#include <stddef.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <math.h>
#include "wait.h"

typedef double field_t;
typedef field_t func_t(field_t);

typedef struct {
    func_t *func;
    field_t left_bound;
    field_t right_bound;
    _Atomic(field_t) result;
    atomic_size_t active_threads;
    atomic_size_t completed_threads;
    size_t threads_num;
    pthread_t *threads;
} par_integrator_t;

static void *integrate_thread(void *arg) {
    par_integrator_t *self = (par_integrator_t *)arg;

    size_t thread_id = atomic_fetch_sub(&self->active_threads, 1) - 1;
    field_t range = self->right_bound - self->left_bound;
    field_t step = range / (field_t)self->threads_num;
    field_t local_start = self->left_bound + step * thread_id;
    field_t local_end = local_start + step;

    field_t local_result = 0;
    const size_t sub_intervals = 1000; // Increase sub_intervals for finer integration
    field_t sub_step = (local_end - local_start) / sub_intervals;

    for (size_t i = 0; i < sub_intervals; ++i) {
        field_t x1 = local_start + i * sub_step;
        field_t x2 = x1 + sub_step;
        local_result += 0.5 * (self->func(x1) + self->func(x2)) * sub_step;
    }

    field_t old_result = atomic_load(&self->result);
    while (!atomic_compare_exchange_weak(&self->result, &old_result, old_result + local_result));

    if (atomic_fetch_add(&self->completed_threads, 1) + 1 == self->threads_num) {
        // Notify main thread when all threads complete
        atomic_notify_all(&self->completed_threads);
    }

    return NULL;
}

int par_integrator_init(par_integrator_t *self, size_t threads_num) {
    if (!self || threads_num == 0) return -1;

    self->threads = malloc(threads_num * sizeof(pthread_t));
    if (!self->threads) return -1;

    self->threads_num = threads_num;
    atomic_store(&self->result, 0.0);
    atomic_store(&self->active_threads, threads_num);
    atomic_store(&self->completed_threads, 0);
    self->func = NULL;
    self->left_bound = 0;
    self->right_bound = 0;

    return 0;
}

int par_integrator_start_calc(par_integrator_t *self, func_t *func, field_t left_bound, field_t right_bound) {
    if (!self || !func || left_bound >= right_bound) return -1;

    self->func = func;
    self->left_bound = left_bound;
    self->right_bound = right_bound;
    atomic_store(&self->result, 0.0);
    atomic_store(&self->active_threads, self->threads_num);
    atomic_store(&self->completed_threads, 0);

    for (size_t i = 0; i < self->threads_num; ++i) {
        if (pthread_create(&self->threads[i], NULL, integrate_thread, self) != 0) {
            // Rollback and join any previously created threads on failure
            for (size_t j = 0; j < i; ++j) {
                pthread_join(self->threads[j], NULL);
            }
            return -1;
        }
    }

    return 0;
}

int par_integrator_get_result(par_integrator_t *self, field_t *result) {
    if (!self || !result) return -1;

    while (atomic_load(&self->completed_threads) < self->threads_num) {
        atomic_wait(&self->completed_threads, atomic_load(&self->completed_threads));
    }

    *result = atomic_load(&self->result);

    return 0;
}

int par_integrator_destroy(par_integrator_t *self) {
    if (!self) return -1;

    for (size_t i = 0; i < self->threads_num; ++i) {
        pthread_join(self->threads[i], NULL);
    }

    free(self->threads);
    self->threads = NULL;

    return 0;
}
