/*
 * Simulacion de un sistema operativo para actividades academicas.
 *
 * El programa registra cada aplicacion abierta como un proceso (con PID y
 * estado), permite crear documentos y carpetas virtuales (solo en memoria,
 * sin tocar el disco), consultar procesos y cerrar programas.
 * Un programa cerrado (TERMINADO) no puede volver a usarse sin abrirlo de nuevo.
 */

#include <iostream>
#include <string>
#include <vector>
#include <limits>

using namespace std;

// ---------------------------------------------------------------------------
// Estructuras de datos
// ---------------------------------------------------------------------------

struct Proceso {
    int pid;            // Identificador unico del proceso
    string nombre;      // Nombre de la aplicacion
    string estado;      // EN_EJECUCION o TERMINADO
};

struct Documento {
    string nombre;
    string contenido;
    string carpeta;     // Carpeta donde esta guardado ("" = raiz)
    int pidCreador;     // Proceso que lo creo
};

struct Carpeta {
    string nombre;
};

// ---------------------------------------------------------------------------
// Sistema operativo simulado
// ---------------------------------------------------------------------------

class SistemaOperativo {
private:
    vector<Proceso> procesos;
    vector<Documento> documentos;
    vector<Carpeta> carpetas;
    int siguientePid;

    Proceso* buscarProceso(int pid) {
        for (size_t i = 0; i < procesos.size(); i++) {
            if (procesos[i].pid == pid) return &procesos[i];
        }
        return NULL;
    }

    bool existeCarpeta(const string& nombre) const {
        for (size_t i = 0; i < carpetas.size(); i++) {
            if (carpetas[i].nombre == nombre) return true;
        }
        return false;
    }

    Documento* buscarDocumento(const string& nombre) {
        for (size_t i = 0; i < documentos.size(); i++) {
            if (documentos[i].nombre == nombre) return &documentos[i];
        }
        return NULL;
    }

public:
    SistemaOperativo() : siguientePid(100) {}

    // Requerimiento 1: abrir el editor de texto y registrarlo como proceso
    int abrirPrograma(const string& nombre) {
        Proceso p;
        p.pid = siguientePid++;
        p.nombre = nombre;
        p.estado = "EN_EJECUCION";
        procesos.push_back(p);
        cout << ">> Programa \"" << nombre << "\" abierto con PID " << p.pid << ".\n";
        return p.pid;
    }

    // Requerimiento 2 y 7: crear un documento solo si el proceso esta activo
    bool crearDocumento(int pid, const string& nombre, const string& contenido,
                        const string& carpeta) {
        Proceso* p = buscarProceso(pid);
        if (p == NULL) {
            cout << ">> Error: no existe un proceso con PID " << pid << ".\n";
            return false;
        }
        if (p->estado == "TERMINADO") {
            cout << ">> Error: el programa \"" << p->nombre << "\" (PID " << pid
                 << ") esta TERMINADO. Debe abrirlo nuevamente para usarlo.\n";
            return false;
        }
        if (buscarDocumento(nombre) != NULL) {
            cout << ">> Error: ya existe un documento llamado \"" << nombre << "\".\n";
            return false;
        }
        if (!carpeta.empty() && !existeCarpeta(carpeta)) {
            cout << ">> Error: la carpeta \"" << carpeta << "\" no existe.\n";
            return false;
        }
        Documento d;
        d.nombre = nombre;
        d.contenido = contenido;
        d.carpeta = carpeta;
        d.pidCreador = pid;
        documentos.push_back(d);
        cout << ">> Documento \"" << nombre << "\" creado con el proceso PID " << pid << ".\n";
        return true;
    }

    // Requerimiento 3: crear una carpeta virtual
    bool crearCarpeta(const string& nombre) {
        if (nombre.empty()) {
            cout << ">> Error: el nombre de la carpeta no puede estar vacio.\n";
            return false;
        }
        if (existeCarpeta(nombre)) {
            cout << ">> Error: la carpeta \"" << nombre << "\" ya existe.\n";
            return false;
        }
        Carpeta c;
        c.nombre = nombre;
        carpetas.push_back(c);
        cout << ">> Carpeta \"" << nombre << "\" creada.\n";
        return true;
    }

    // Organizar: mover un documento existente a una carpeta
    bool moverDocumento(const string& documento, const string& carpeta) {
        Documento* d = buscarDocumento(documento);
        if (d == NULL) {
            cout << ">> Error: no existe el documento \"" << documento << "\".\n";
            return false;
        }
        if (!existeCarpeta(carpeta)) {
            cout << ">> Error: la carpeta \"" << carpeta << "\" no existe.\n";
            return false;
        }
        d->carpeta = carpeta;
        cout << ">> Documento \"" << documento << "\" movido a la carpeta \"" << carpeta << "\".\n";
        return true;
    }

    // Requerimiento 4: mostrar documentos y carpetas registrados
    void mostrarArchivos() const {
        cout << "\n========== ARCHIVOS REGISTRADOS ==========\n";
        if (carpetas.empty() && documentos.empty()) {
            cout << "No hay carpetas ni documentos registrados.\n";
            return;
        }
        for (size_t i = 0; i < carpetas.size(); i++) {
            cout << "[Carpeta] " << carpetas[i].nombre << "/\n";
            bool vacia = true;
            for (size_t j = 0; j < documentos.size(); j++) {
                if (documentos[j].carpeta == carpetas[i].nombre) {
                    cout << "    - " << documentos[j].nombre
                         << " (PID creador: " << documentos[j].pidCreador << ")\n"
                         << "      Contenido: " << documentos[j].contenido << "\n";
                    vacia = false;
                }
            }
            if (vacia) cout << "    (vacia)\n";
        }
        bool hayRaiz = false;
        for (size_t j = 0; j < documentos.size(); j++) {
            if (documentos[j].carpeta.empty()) {
                if (!hayRaiz) {
                    cout << "[Sin carpeta]\n";
                    hayRaiz = true;
                }
                cout << "    - " << documentos[j].nombre
                     << " (PID creador: " << documentos[j].pidCreador << ")\n"
                     << "      Contenido: " << documentos[j].contenido << "\n";
            }
        }
    }

    // Requerimiento 5: consultar procesos activos (y el historial completo)
    void mostrarProcesos() const {
        cout << "\n============ TABLA DE PROCESOS ============\n";
        if (procesos.empty()) {
            cout << "No se ha abierto ningun programa.\n";
            return;
        }
        int activos = 0;
        cout << "PID\tESTADO\t\tPROGRAMA\n";
        for (size_t i = 0; i < procesos.size(); i++) {
            cout << procesos[i].pid << "\t" << procesos[i].estado << "\t"
                 << procesos[i].nombre << "\n";
            if (procesos[i].estado == "EN_EJECUCION") activos++;
        }
        cout << "Procesos activos: " << activos << "\n";
    }

    // Requerimiento 6: cerrar un programa y cambiar su estado a TERMINADO
    bool cerrarPrograma(int pid) {
        Proceso* p = buscarProceso(pid);
        if (p == NULL) {
            cout << ">> Error: no existe un proceso con PID " << pid << ".\n";
            return false;
        }
        if (p->estado == "TERMINADO") {
            cout << ">> El proceso PID " << pid << " ya estaba TERMINADO.\n";
            return false;
        }
        p->estado = "TERMINADO";
        cout << ">> Programa \"" << p->nombre << "\" (PID " << pid << ") cerrado. Estado: TERMINADO.\n";
        return true;
    }

    // Al apagar el sistema se cierran todos los programas que sigan abiertos
    void cerrarTodo() {
        for (size_t i = 0; i < procesos.size(); i++) {
            if (procesos[i].estado == "EN_EJECUCION") cerrarPrograma(procesos[i].pid);
        }
    }
};

// ---------------------------------------------------------------------------
// Funciones auxiliares de entrada
// ---------------------------------------------------------------------------

int leerEntero(const string& mensaje) {
    int valor;
    while (true) {
        cout << mensaje;
        if (cin >> valor) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return valor;
        }
        if (cin.eof()) return 0;
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Entrada invalida, ingrese un numero.\n";
    }
}

string leerTexto(const string& mensaje) {
    string texto;
    cout << mensaje;
    getline(cin, texto);
    return texto;
}

// ---------------------------------------------------------------------------
// Programa principal
// ---------------------------------------------------------------------------

int main() {
    SistemaOperativo so;
    int opcion;

    do {
        cout << "\n======= SIMULADOR DE SISTEMA OPERATIVO =======\n"
             << "1. Abrir editor de texto\n"
             << "2. Crear documento\n"
             << "3. Crear carpeta\n"
             << "4. Mover documento a una carpeta\n"
             << "5. Mostrar documentos y carpetas\n"
             << "6. Consultar procesos\n"
             << "7. Cerrar un programa\n"
             << "0. Apagar (cerrar todo y salir)\n";
        opcion = leerEntero("Seleccione una opcion: ");

        switch (opcion) {
        case 1:
            so.abrirPrograma("Editor de texto");
            break;
        case 2: {
            so.mostrarProcesos();
            int pid = leerEntero("PID del editor a utilizar: ");
            string nombre = leerTexto("Nombre del documento: ");
            string contenido = leerTexto("Contenido: ");
            string carpeta = leerTexto("Carpeta (Enter para dejarlo sin carpeta): ");
            so.crearDocumento(pid, nombre, contenido, carpeta);
            break;
        }
        case 3:
            so.crearCarpeta(leerTexto("Nombre de la carpeta: "));
            break;
        case 4: {
            string documento = leerTexto("Nombre del documento: ");
            string carpeta = leerTexto("Carpeta destino: ");
            so.moverDocumento(documento, carpeta);
            break;
        }
        case 5:
            so.mostrarArchivos();
            break;
        case 6:
            so.mostrarProcesos();
            break;
        case 7:
            so.cerrarPrograma(leerEntero("PID del programa a cerrar: "));
            break;
        case 0:
            cout << "\nApagando el sistema...\n";
            so.cerrarTodo();
            so.mostrarProcesos();
            cout << "Hasta luego.\n";
            break;
        default:
            cout << "Opcion no valida.\n";
        }
    } while (opcion != 0);

    return 0;
}
