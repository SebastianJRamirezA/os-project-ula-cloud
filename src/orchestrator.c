#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "orchestrator.h"
#include <sys/resource.h>

/**
 * TODO: Implementar el despliegue del microservicio.
 * * Requisitos:
 * 1. Crear un nuevo proceso.
 * 2. En el proceso HIJO:
 * - Configurar las restricciones de recursos (memoria).
 * - Reemplazar la imagen del proceso por el binario del servicio.
 * 3. En el proceso PADRE:
 * - Registrar el PID y el estado inicial en el dashboard.
 * - Retornar el PID asignado.
 */
int spawn_service(int index) {
    pid_t pid = fork();

    // TODO: Invocar la creación del proceso hijo.

    // Casos a manejar:
    // - Error en la creación del proceso.
    // - Lógica del proceso HIJO (Setup de límites y Ejecución).
    // - Lógica del proceso PADRE (Gestión del dashboard).

    // - Error en la creación del proceso.
    if (pid < 0) {
        dashboard[index].state = STATE_CRASHED;
        return -1; // Error al crear el proceso
    } 
    // - Lógica del proceso HIJO (Setup de límites y Ejecución).
    else if (pid == 0) {

        struct rlimit limit;
        limit.rlim_cur = dashboard[index].mem_limit; 
        limit.rlim_max = dashboard[index].mem_limit;

        if (setrlimit(RLIMIT_AS, &limit) == -1) {
            perror("Error al establecer límite");
            exit(EXIT_FAILURE);
        }
        
        char *args[] = {dashboard[index].path, NULL}; 
        execvp(args[0], args); 

        // Si execv retorna, es que hubo un error
        dashboard[index].state = STATE_CRASHED;
        exit(EXIT_FAILURE);
    } 
    // - Lógica del proceso PADRE (Gestión del dashboard).
    else {
        dashboard[index].pid = pid;
        dashboard[index].state = STATE_RUNNING;
    }

    return pid; // Cambiar por el PID real
}
