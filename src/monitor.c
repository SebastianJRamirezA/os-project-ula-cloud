#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <pthread.h>
#include "orchestrator.h"

/**
 * TODO: Implementar la rutina del hilo monitor (Watchdog).
 * * Requisitos:
 * 1. Recuperar la estructura del servicio desde el argumento 'arg'.
 * 2. Utilizar 'waitpid' de forma BLOQUEANTE para esperar al proceso hijo.
 * 3. Analizar el estado de terminación (éxito, error o señal del sistema).
 * 4. Actualizar el Dashboard Global de forma SEGURA (evitar condiciones de carrera).
 */
void* monitor_service(void *arg) {
    // TODO: Castear el argumento al tipo de dato correcto.
    service_t *service = (service_t *)arg;
    
    // TODO: Implementar la espera del proceso específico.
    // Ayuda: Revisar el uso de waitpid(pid, &status, 0).
    int status;
    waitpid(service->pid, &status, 0);

    /* * Una vez que waitpid retorna, el proceso hijo ha cambiado de estado.
     * TODO: Analizar el 'status' usando las macros de sys/wait.h:
     * - WIFEXITED: ¿Terminó normalmente?
     * - WEXITSTATUS: ¿Cuál fue su código de retorno?
     * - WIFSIGNALED: ¿Fue terminado por una señal (Segfault, OOM Killer)?
     * - WTERMSIG: ¿Qué señal lo mató?
     */

     if (WIFEXITED(status)) {
        service->state = (WEXITSTATUS(status) == 0) ? STATE_STOPPED : STATE_CRASHED;
        service->exit_status = WEXITSTATUS(status);
     } else if (WIFSIGNALED(status)) {
        service->state = STATE_KILLED;
        service->exit_status = WTERMSIG(status);
     } else {
        service->state = STATE_CRASHED; // Estado indeterminado, marcar como CRASHED
        service->exit_status = -1; // Código de salida desconocido
     }

    /*
     * TODO: Actualizar el dashboard global.
     * ¡CRÍTICO!: El acceso al array 'dashboard' debe estar protegido. 
     * No olvides liberar el mecanismo de sincronización al terminar.
     */

    return NULL;
}
