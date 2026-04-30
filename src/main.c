#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include "orchestrator.h"

/* --- Variables Globales --- */
service_t dashboard[MAX_SERVICES];
int num_services = 0;
pthread_mutex_t dashboard_mutex;

/**
 * Función de utilidad para limpiar la terminal.
 */
void clear_screen() {
    printf("\033[H\033[J");
}

/**
 * TODO: Implementar la visualización del estado actual de los servicios.
 * Se debe garantizar una lectura consistente de los datos compartidos.
 */
void print_dashboard() {
    clear_screen();
    printf("==============================================================\n");
    printf("                ULA-CLOUD MONITORING DASHBOARD               \n");
    printf("==============================================================\n");
    printf("%-15s %-10s %-15s %-10s\n", "SERVICIO", "PID", "ESTADO", "EXIT/SIG");
    printf("--------------------------------------------------------------\n");

    // TODO: Renderizar cada fila del dashboard con la información actualizada.
    pthread_mutex_lock(&dashboard_mutex);
    for(int i = 0; i < num_services; i++) {
        service_t svc = dashboard[i];

        const char *state_str;
        switch (svc.state) {
            case STATE_IDLE: state_str = "IDLE"; break;
            case STATE_RUNNING: state_str = "RUNNING"; break;
            case STATE_CRASHED: state_str = "CRASHED"; break;
            case STATE_KILLED: state_str = "KILLED"; break;
            case STATE_STOPPED: state_str = "STOPPED"; break;
            default: state_str = "UNKNOWN"; break;
        }

        printf("%-15s %-10d %-15s %-10d\n", svc.name, svc.pid, state_str, svc.exit_status);
    }
    pthread_mutex_unlock(&dashboard_mutex);

    printf("==============================================================\n");
}

/**
 * TODO: Gestión de finalización del orquestador.
 * Implementar una estrategia para evitar la proliferación de procesos huérfanos.
 */
void handle_shutdown(int sig) {
    printf("\n[ULA-Cloud] Iniciando secuencia de apagado...\n");
    
    // TODO: Notificar y limpiar recursos de procesos hijos.
    pthread_mutex_lock(&dashboard_mutex);
    for (int i = 0; i < num_services; i++) {
        if (dashboard[i].state == STATE_RUNNING) {
            printf("Terminando servicio %s (PID %d)...\n", dashboard[i].name, dashboard[i].pid);
            kill(dashboard[i].pid, SIGTERM); // Enviar señal de terminación
        }
    }
    pthread_mutex_unlock(&dashboard_mutex);

    for (int i = 0; i < num_services; i++) {
        pthread_join(dashboard[i].monitor_thread, NULL); // Esperar a que el hilo monitor termine
    }
    
    exit(sig);
}

int main(int argc, char *argv[]) {
    // 1. Inicialización de mecanismos de sincronización
    if (pthread_mutex_init(&dashboard_mutex, NULL) != 0) {
        perror("Error inicializando mutex");
        return 1;
    }

    // 2. Captura de interrupciones del sistema
    signal(SIGINT, handle_shutdown);

    // 3. Configuración de la carga de trabajo (Servicios de prueba)
    num_services = 3;
    
    strcpy(dashboard[0].name, "Logger");
    strcpy(dashboard[0].path, "./bin/logger");
    dashboard[0].mem_limit = DEFAULT_MEM_LIMIT;

    strcpy(dashboard[1].name, "Chaos");
    strcpy(dashboard[1].path, "./bin/chaos");
    dashboard[1].mem_limit = DEFAULT_MEM_LIMIT;

    strcpy(dashboard[2].name, "Leak");
    strcpy(dashboard[2].path, "./bin/leak");
    dashboard[2].mem_limit = 20 * 1024 * 1024; // Límite de 20MB

    if (argc > 1 && strcmp(argv[1], "--mem") ==0) {
        if (argc < 3) {
            fprintf(stderr, "Uso: %s --mem <MB>\n", argv[0]);
            return 1;
        }

        char *endptr;
        errno = 0;
        unsigned long mem_mb = strtoul(argv[2], &endptr, 10);

        // Validación de errores de strtoul
        if (errno == ERANGE || *endptr != '\0' || mem_mb == 0 || mem_mb > INT_MAX) {
            fprintf(stderr, "Error: El límite de memoria '%s' no es un valor válido.\n", argv[2]);
            return 1;
        }

        size_t mem_limit = mem_mb * 1024 * 1024; // Convertir MB a Bytes
        for (int i = 0; i < num_services; i++) {
            dashboard[i].mem_limit = mem_limit;
        }
    }

    // 4. Activación del ecosistema
    printf("[ULA-Cloud] Inicializando %d microservicios...\n", num_services);
    
    for (int i = 0; i < num_services; i++) {
        /* * TODO: Orquestar el despliegue de servicios y su posterior 
         * monitoreo concurrente. 
         */
        int pid = spawn_service(i);
        if (pid < 0) {
            fprintf(stderr, "Error al lanzar el servicio %s\n", dashboard[i].name);
        } else {
            printf("Servicio %s lanzado con PID %d\n", dashboard[i].name, pid);
        }
    }

    // 5. Ciclo de monitoreo principal
    while (1) {
        print_dashboard();
        sleep(1); 
    }

    pthread_mutex_destroy(&dashboard_mutex);
    return 0;
}
