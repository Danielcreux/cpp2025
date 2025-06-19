#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <algorithm>
#include <sqlite3.h>
#include <windows.h>
#include <fstream>
#include <iomanip>
#include <sstream>

using namespace std;

// Definiciones de colores ANSI para Windows
const string RESET = "\033[0m";
const string RED = "\033[31m";
const string GREEN = "\033[32m";
const string YELLOW = "\033[33m";
const string BLUE = "\033[34m";
const string MAGENTA = "\033[35m";
const string CYAN = "\033[36m";
const string WHITE = "\033[37m";
const string BOLD = "\033[1m";
const string BG_BLUE = "\033[44m";
const string BG_GREEN = "\033[42m";
const string BG_RED = "\033[41m";
const string BG_YELLOW = "\033[43m";
const string BG_MAGENTA = "\033[45m";

sqlite3* db;

// Función para establecer el título de la consola
void setConsoleTitle(const string& title) {
    SetConsoleTitleA(title.c_str());
}

// Función para cambiar el color del texto de la consola
void setConsoleColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

// Función para limpiar la pantalla
void clearScreen() {
    system("cls");
}

// Función para dibujar una línea decorativa
void drawLine(int length, char symbol, const string& color) {
    cout << color;
    for (int i = 0; i < length; i++) {
        cout << symbol;
    }
    cout << RESET << endl;
}

// Función para mostrar un encabezado
void showHeader(const string& title) {
    clearScreen();
    setConsoleTitle("SQLite CRUD Manager - " + title);
    
    drawLine(80, '=', BOLD + BG_BLUE + WHITE);
    cout << BOLD + BG_BLUE + WHITE;
    cout << "  " << title << RESET << endl;
    drawLine(80, '=', BOLD + BG_BLUE + WHITE);
    cout << endl;
}

// Función para verificar si una tabla existe
bool tableExists(const string& tableName) {
    string sql = "SELECT count(*) FROM sqlite_master WHERE type='table' AND name=?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << RED << "Error al preparar la consulta: " << sqlite3_errmsg(db) << RESET << endl;
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, tableName.c_str(), -1, SQLITE_STATIC);
    
    bool exists = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        exists = (sqlite3_column_int(stmt, 0) > 0);
    }
    
    sqlite3_finalize(stmt);
    return exists;
}

// Función para crear una tabla
void createTable() {
    showHeader("CREAR TABLA");
    
    string tableName;
    cout << CYAN << "Nombre de la tabla: " << RESET;
    getline(cin, tableName);

    if (tableExists(tableName)) {
        cout << BG_RED << WHITE << " Error: La tabla ya existe! " << RESET << endl;
        Sleep(2000);
        return;
    }

    int colCount;
    cout << CYAN << "Numero de columnas: " << RESET;
    cin >> colCount;
    cin.ignore();

    vector<pair<string, string>> columns;
    for (int i = 0; i < colCount; i++) {
        string name, type;
        cout << YELLOW << "\nColumna " << i+1 << RESET << endl;
        cout << CYAN << "  Nombre: " << RESET;
        getline(cin, name);
        
        cout << CYAN << "  Tipo (TEXT/INTEGER/REAL/BLOB): " << RESET;
        getline(cin, type);
        transform(type.begin(), type.end(), type.begin(), ::toupper);
        
        if (type != "TEXT" && type != "INTEGER" && type != "REAL" && type != "BLOB") {
            cout << RED << "  Tipo invalido. Usando TEXT por defecto." << RESET << endl;
            type = "TEXT";
        }
        columns.push_back({name, type});
    }

    string sql = "CREATE TABLE \"" + tableName + "\" (";
    for (size_t i = 0; i < columns.size(); i++) {
        sql += "\"" + columns[i].first + "\" " + columns[i].second;
        if (i < columns.size() - 1) sql += ", ";
    }
    sql += ");";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    
    if (rc != SQLITE_OK) {
        cout << BG_RED << WHITE << " Error SQL: " << (errMsg ? errMsg : "Error desconocido") << RESET << endl;
        if(errMsg) sqlite3_free(errMsg);
        Sleep(3000);
    } else {
        cout << BG_GREEN << WHITE << " Tabla creada exitosamente! " << RESET << endl;
        Sleep(1500);
    }
}

// Función para eliminar una tabla
void deleteTable() {
    showHeader("ELIMINAR TABLA");
    
    string tableName;
    cout << CYAN << "Nombre de tabla a eliminar: " << RESET;
    getline(cin, tableName);

    if (!tableExists(tableName)) {
        cout << BG_RED << WHITE << " Error: La tabla no existe! " << RESET << endl;
        Sleep(2000);
        return;
    }

    string sql = "DROP TABLE \"" + tableName + "\";";
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    
    if (rc != SQLITE_OK) {
        cout << BG_RED << WHITE << " Error SQL: " << (errMsg ? errMsg : "Error desconocido") << RESET << endl;
        if(errMsg) sqlite3_free(errMsg);
        Sleep(3000);
    } else {
        cout << BG_GREEN << WHITE << " Tabla eliminada exitosamente! " << RESET << endl;
        Sleep(1500);
    }
}

// Función para listar tablas
void listTables() {
    showHeader("TABLAS EXISTENTES");
    
    const char* sql = "SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%';";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cout << BG_RED << WHITE << " Error al listar tablas: " << sqlite3_errmsg(db) << RESET << endl;
        Sleep(3000);
        return;
    }
    
    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        count++;
        cout << GREEN << " - " << CYAN << sqlite3_column_text(stmt, 0) << RESET << endl;
    }
    
    sqlite3_finalize(stmt);
    
    if (count == 0) {
        cout << YELLOW << " No hay tablas en la base de datos " << RESET << endl;
    }
    
    cout << endl << MAGENTA << "Total de tablas: " << count << RESET << endl;
    cout << endl << CYAN << "Presione Enter para continuar..." << RESET;
    cin.ignore();
}

// Función para mostrar datos de una tabla (con paginación)
void showTableData(const string& tableName, int page = 1, int pageSize = 10) {
    int offset = (page - 1) * pageSize;
    string sql = "SELECT * FROM \"" + tableName + "\" LIMIT " + to_string(pageSize) + " OFFSET " + to_string(offset) + ";";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cout << BG_RED << WHITE << " Error al preparar consulta: " << sqlite3_errmsg(db) << RESET << endl;
        return;
    }
    
    // Obtener nombres de columnas
    int colCount = sqlite3_column_count(stmt);
    vector<string> colNames;
    for (int i = 0; i < colCount; i++) {
        colNames.push_back(sqlite3_column_name(stmt, i));
    }
    
    // Calcular anchos de columnas
    vector<int> colWidths(colCount, 15);
    for (int i = 0; i < colCount; i++) {
        if (colNames[i].length() + 2 > colWidths[i]) {
            colWidths[i] = colNames[i].length() + 2;
        }
    }
    
    // Imprimir encabezados
    cout << BG_BLUE << WHITE;
    for (int i = 0; i < colCount; i++) {
        cout << setw(colWidths[i]) << left << colNames[i];
    }
    cout << RESET << endl;
    
    // Imprimir datos
    int rowCount = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        rowCount++;
        for (int i = 0; i < colCount; i++) {
            const char* val = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            if (val) {
                cout << setw(colWidths[i]) << left << val;
            } else {
                cout << setw(colWidths[i]) << left << "NULL";
            }
        }
        cout << endl;
    }
    
    sqlite3_finalize(stmt);
    
    // Contar total de filas
    string countSql = "SELECT COUNT(*) FROM \"" + tableName + "\";";
    int totalRows = 0;
    if (sqlite3_prepare_v2(db, countSql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            totalRows = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    
    // Mostrar paginación
    int totalPages = (totalRows + pageSize - 1) / pageSize;
    cout << MAGENTA << "\nPágina " << page << " de " << totalPages;
    cout << " | Registros: " << rowCount << " de " << totalRows;
    cout << " | Tamaño página: " << pageSize << RESET << endl;
}

// Función para ver datos de una tabla con navegación
void viewTableData() {
    showHeader("VER DATOS DE TABLA");
    
    const char* sql = "SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%';";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cout << BG_RED << WHITE << " Error al obtener tablas: " << sqlite3_errmsg(db) << RESET << endl;
        Sleep(3000);
        return;
    }
    
    vector<string> tables;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        tables.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
    }
    sqlite3_finalize(stmt);
    
    if (tables.empty()) {
        cout << BG_RED << WHITE << " No hay tablas disponibles! " << RESET << endl;
        Sleep(2000);
        return;
    }
    
    cout << YELLOW << "Tablas disponibles:" << RESET << endl;
    for (size_t i = 0; i < tables.size(); i++) {
        cout << GREEN << " " << i+1 << ". " << CYAN << tables[i] << RESET << endl;
    }
    
    int tableChoice;
    cout << CYAN << "\nSeleccione una tabla (1-" << tables.size() << "): " << RESET;
    cin >> tableChoice;
    cin.ignore();
    
    if (tableChoice < 1 || tableChoice > static_cast<int>(tables.size())) {
        cout << BG_RED << WHITE << " Seleccion invalida! " << RESET << endl;
        Sleep(2000);
        return;
    }
    
    string tableName = tables[tableChoice - 1];
    
    // Navegación por páginas
    int page = 1;
    int pageSize = 10;
    bool viewing = true;
    
    while (viewing) {
        showHeader("TABLA: " + tableName);
        showTableData(tableName, page, pageSize);
        
        cout << "\nOpciones:\n";
        cout << GREEN << "  N: " << RESET << "Siguiente página\n";
        cout << GREEN << "  P: " << RESET << "Página anterior\n";
        cout << GREEN << "  S: " << RESET << "Cambiar tamaño de página\n";
        cout << GREEN << "  B: " << RESET << "Buscar en esta tabla\n";
        cout << GREEN << "  X: " << RESET << "Eliminar registros\n";
        cout << GREEN << "  E: " << RESET << "Exportar datos\n";
        cout << GREEN << "  V: " << RESET << "Volver al menú\n";
        cout << CYAN << "Seleccion: " << RESET;
        
        char choice;
        cin >> choice;
        cin.ignore();
        
        switch (toupper(choice)) {
            case 'N':
                page++;
                break;
            case 'P':
                if (page > 1) page--;
                break;
            case 'S': {
                cout << CYAN << "Nuevo tamaño de página: " << RESET;
                int newSize;
                cin >> newSize;
                cin.ignore();
                if (newSize > 0) pageSize = newSize;
                break;
            }
            case 'B':
                // Buscar en la tabla
                break;
            case 'X':
                // Eliminar registros
                break;
            case 'E': {
                // Exportar datos
                string filename = tableName + "_export.csv";
                ofstream file(filename);
                
                if (!file.is_open()) {
                    cout << BG_RED << WHITE << " Error al crear archivo! " << RESET << endl;
                    Sleep(2000);
                    break;
                }
                
                // Obtener nombres de columnas
                string pragma = "PRAGMA table_info(\"" + tableName + "\");";
                vector<string> columns;
                if (sqlite3_prepare_v2(db, pragma.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    while (sqlite3_step(stmt) == SQLITE_ROW) {
                        columns.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
                    }
                    sqlite3_finalize(stmt);
                }
                
                // Escribir encabezados
                for (size_t i = 0; i < columns.size(); i++) {
                    file << columns[i];
                    if (i < columns.size() - 1) file << ",";
                }
                file << endl;
                
                // Escribir datos
                string dataSql = "SELECT * FROM \"" + tableName + "\";";
                if (sqlite3_prepare_v2(db, dataSql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    while (sqlite3_step(stmt) == SQLITE_ROW) {
                        for (int i = 0; i < sqlite3_column_count(stmt); i++) {
                            const char* val = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                            if (val) {
                                // Escapar comas
                                string value = val;
                                if (value.find(',') != string::npos) {
                                    value = "\"" + value + "\"";
                                }
                                file << value;
                            }
                            if (i < sqlite3_column_count(stmt) - 1) file << ",";
                        }
                        file << endl;
                    }
                    sqlite3_finalize(stmt);
                }
                
                file.close();
                cout << BG_GREEN << WHITE << " Datos exportados a " << filename << " " << RESET << endl;
                Sleep(2000);
                break;
            }
            case 'V':
                viewing = false;
                break;
            default:
                cout << BG_RED << WHITE << " Opcion invalida! " << RESET << endl;
                Sleep(1000);
        }
    }
}

// Función para insertar datos
void insertData() {
    showHeader("INSERTAR DATOS");
    
    const char* sql = "SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%';";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cout << BG_RED << WHITE << " Error al obtener tablas: " << sqlite3_errmsg(db) << RESET << endl;
        Sleep(3000);
        return;
    }
    
    vector<string> tables;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        tables.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
    }
    sqlite3_finalize(stmt);
    
    if (tables.empty()) {
        cout << BG_RED << WHITE << " No hay tablas disponibles! " << RESET << endl;
        Sleep(2000);
        return;
    }
    
    cout << YELLOW << "Tablas disponibles:" << RESET << endl;
    for (size_t i = 0; i < tables.size(); i++) {
        cout << GREEN << " " << i+1 << ". " << CYAN << tables[i] << RESET << endl;
    }
    
    int tableChoice;
    cout << CYAN << "\nSeleccione una tabla (1-" << tables.size() << "): " << RESET;
    cin >> tableChoice;
    cin.ignore();
    
    if (tableChoice < 1 || tableChoice > static_cast<int>(tables.size())) {
        cout << BG_RED << WHITE << " Seleccion invalida! " << RESET << endl;
        Sleep(2000);
        return;
    }
    
    string tableName = tables[tableChoice - 1];
    
    // Obtener columnas
    string pragma = "PRAGMA table_info(\"" + tableName + "\");";
    vector<string> columns;
    if (sqlite3_prepare_v2(db, pragma.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cout << BG_RED << WHITE << " Error al obtener columnas: " << sqlite3_errmsg(db) << RESET << endl;
        Sleep(3000);
        return;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        columns.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
    }
    sqlite3_finalize(stmt);
    
    if (columns.empty()) {
        cout << BG_RED << WHITE << " La tabla no tiene columnas! " << RESET << endl;
        Sleep(2000);
        return;
    }
    
    // Construir INSERT
    string insertSql = "INSERT INTO \"" + tableName + "\" (";
    for (size_t i = 0; i < columns.size(); i++) {
        insertSql += "\"" + columns[i] + "\"";
        if (i < columns.size() - 1) insertSql += ", ";
    }
    insertSql += ") VALUES (";
    for (size_t i = 0; i < columns.size(); i++) {
        insertSql += "?";
        if (i < columns.size() - 1) insertSql += ", ";
    }
    insertSql += ");";

    // Preparar statement
    if (sqlite3_prepare_v2(db, insertSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cout << BG_RED << WHITE << " Error al preparar INSERT: " << sqlite3_errmsg(db) << RESET << endl;
        Sleep(3000);
        return;
    }

    // Vincular valores
    for (size_t i = 0; i < columns.size(); i++) {
        string value;
        cout << CYAN << "Valor para " << columns[i] << ": " << RESET;
        getline(cin, value);
        sqlite3_bind_text(stmt, i+1, value.c_str(), -1, SQLITE_TRANSIENT);
    }

    // Ejecutar
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        cout << BG_RED << WHITE << " Error al insertar: " << sqlite3_errmsg(db) << RESET << endl;
        Sleep(3000);
    } else {
        cout << BG_GREEN << WHITE << " Datos insertados exitosamente! " << RESET << endl;
        Sleep(1500);
    }
    sqlite3_finalize(stmt);
}

// Función para actualizar datos
void updateData() {
    showHeader("ACTUALIZAR DATOS");
    
    const char* sql = "SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%';";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cout << BG_RED << WHITE << " Error al obtener tablas: " << sqlite3_errmsg(db) << RESET << endl;
        Sleep(3000);
        return;
    }
    
    vector<string> tables;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        tables.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
    }
    sqlite3_finalize(stmt);
    
    if (tables.empty()) {
        cout << BG_RED << WHITE << " No hay tablas disponibles! " << RESET << endl;
        Sleep(2000);
        return;
    }
    
    cout << YELLOW << "Tablas disponibles:" << RESET << endl;
    for (size_t i = 0; i < tables.size(); i++) {
        cout << GREEN << " " << i+1 << ". " << CYAN << tables[i] << RESET << endl;
    }
    
    int tableChoice;
    cout << CYAN << "\nSeleccione una tabla (1-" << tables.size() << "): " << RESET;
    cin >> tableChoice;
    cin.ignore();
    
    if (tableChoice < 1 || tableChoice > static_cast<int>(tables.size())) {
        cout << BG_RED << WHITE << " Seleccion invalida! " << RESET << endl;
        Sleep(2000);
        return;
    }
    
    string tableName = tables[tableChoice - 1];
    
    // Mostrar datos actuales
    cout << YELLOW << "\nDatos actuales en la tabla " << tableName << ":" << RESET << endl;
    showTableData(tableName);
    cout << endl;
    
    // Obtener columnas
    string pragma = "PRAGMA table_info(\"" + tableName + "\");";
    vector<string> columns;
    if (sqlite3_prepare_v2(db, pragma.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cout << BG_RED << WHITE << " Error al obtener columnas: " << sqlite3_errmsg(db) << RESET << endl;
        Sleep(3000);
        return;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        columns.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
    }
    sqlite3_finalize(stmt);
    
    if (columns.empty()) {
        cout << BG_RED << WHITE << " La tabla no tiene columnas! " << RESET << endl;
        Sleep(2000);
        return;
    }
    
    // Mostrar columnas
    cout << YELLOW << "Columnas disponibles:" << RESET << endl;
    for (size_t i = 0; i < columns.size(); i++) {
        cout << GREEN << " " << i+1 << ". " << CYAN << columns[i] << RESET << endl;
    }
    
    int updateColChoice, conditionColChoice;
    cout << CYAN << "\nSeleccione columna a actualizar (1-" << columns.size() << "): " << RESET;
    cin >> updateColChoice;
    cin.ignore();
    
    if (updateColChoice < 1 || updateColChoice > static_cast<int>(columns.size())) {
        cout << BG_RED << WHITE << " Seleccion invalida! " << RESET << endl;
        Sleep(2000);
        return;
    }
    
    string updateCol = columns[updateColChoice - 1];
    
    cout << CYAN << "Nuevo valor: " << RESET;
    string newValue;
    getline(cin, newValue);
    
    cout << CYAN << "\nSeleccione columna para condicion WHERE (1-" << columns.size() << "): " << RESET;
    cin >> conditionColChoice;
    cin.ignore();
    
    if (conditionColChoice < 1 || conditionColChoice > static_cast<int>(columns.size())) {
        cout << BG_RED << WHITE << " Seleccion invalida! " << RESET << endl;
        Sleep(2000);
        return;
    }
    
    string conditionCol = columns[conditionColChoice - 1];
    
    cout << CYAN << "Valor para condicion WHERE: " << RESET;
    string conditionValue;
    getline(cin, conditionValue);

    // Construir UPDATE
    string updateSql = "UPDATE \"" + tableName + "\" SET \"" + updateCol + "\" = ? WHERE \"" + conditionCol + "\" = ?;";
    if (sqlite3_prepare_v2(db, updateSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cout << BG_RED << WHITE << " Error al preparar UPDATE: " << sqlite3_errmsg(db) << RESET << endl;
        Sleep(3000);
        return;
    }

    sqlite3_bind_text(stmt, 1, newValue.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, conditionValue.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        cout << BG_RED << WHITE << " Error al actualizar: " << sqlite3_errmsg(db) << RESET << endl;
        Sleep(3000);
    } else {
        int changes = sqlite3_changes(db);
        if (changes > 0) {
            cout << BG_GREEN << WHITE << " " << changes << " registro(s) actualizado(s) exitosamente! " << RESET << endl;
        } else {
            cout << BG_YELLOW << WHITE << " Ningun registro actualizado. Verifique la condicion WHERE. " << RESET << endl;
        }
        Sleep(2000);
    }
    sqlite3_finalize(stmt);
}

// Función para buscar datos
void searchData() {
    showHeader("BUSCAR DATOS");
    
    const char* sql = "SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%';";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        cout << BG_RED << WHITE << " Error al obtener tablas: " << sqlite3_errmsg(db) << RESET << endl;
        Sleep(3000);
        return;
    }
    
    vector<string> tables;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        tables.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
    }
    sqlite3_finalize(stmt);
    
    if (tables.empty()) {
        cout << BG_RED << WHITE << " No hay tablas disponibles! " << RESET << endl;
        Sleep(2000);
        return;
    }
    
    cout << YELLOW << "Tablas disponibles:" << RESET << endl;
    for (size_t i = 0; i < tables.size(); i++) {
        cout << GREEN << " " << i+1 << ". " << CYAN << tables[i] << RESET << endl;
    }
    
    int tableChoice;
    cout << CYAN << "\nSeleccione una tabla (1-" << tables.size() << "): " << RESET;
    cin >> tableChoice;
    cin.ignore();
    
    if (tableChoice < 1 || tableChoice > static_cast<int>(tables.size())) {
        cout << BG_RED << WHITE << " Seleccion invalida! " << RESET << endl;
        Sleep(2000);
        return;
    }
    
    string tableName = tables[tableChoice - 1];
    
    // Obtener columnas
    string pragma = "PRAGMA table_info(\"" + tableName + "\");";
    vector<string> columns;
    if (sqlite3_prepare_v2(db, pragma.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cout << BG_RED << WHITE << " Error al obtener columnas: " << sqlite3_errmsg(db) << RESET << endl;
        Sleep(3000);
        return;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        columns.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
    }
    sqlite3_finalize(stmt);
    
    if (columns.empty()) {
        cout << BG_RED << WHITE << " La tabla no tiene columnas! " << RESET << endl;
        Sleep(2000);
        return;
    }
    
    // Mostrar columnas
    cout << YELLOW << "Columnas disponibles:" << RESET << endl;
    for (size_t i = 0; i < columns.size(); i++) {
        cout << GREEN << " " << i+1 << ". " << CYAN << columns[i] << RESET << endl;
    }
    
    int searchColChoice;
    cout << CYAN << "\nSeleccione columna para buscar (1-" << columns.size() << "): " << RESET;
    cin >> searchColChoice;
    cin.ignore();
    
    if (searchColChoice < 1 || searchColChoice > static_cast<int>(columns.size())) {
        cout << BG_RED << WHITE << " Seleccion invalida! " << RESET << endl;
        Sleep(2000);
        return;
    }
    
    string searchCol = columns[searchColChoice - 1];
    
    cout << CYAN << "Valor a buscar: " << RESET;
    string searchValue;
    getline(cin, searchValue);

    // Construir SELECT
    string selectSql = "SELECT * FROM \"" + tableName + "\" WHERE \"" + searchCol + "\" LIKE ?;";
    if (sqlite3_prepare_v2(db, selectSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cout << BG_RED << WHITE << " Error al preparar busqueda: " << sqlite3_errmsg(db) << RESET << endl;
        Sleep(3000);
        return;
    }

    string wildcardValue = "%" + searchValue + "%";
    sqlite3_bind_text(stmt, 1, wildcardValue.c_str(), -1, SQLITE_TRANSIENT);

    // Mostrar resultados
    cout << YELLOW << "\nResultados de la busqueda:" << RESET << endl;
    
    // Obtener nombres de columnas
    int colCount = sqlite3_column_count(stmt);
    vector<string> colNames;
    for (int i = 0; i < colCount; i++) {
        colNames.push_back(sqlite3_column_name(stmt, i));
    }
    
    // Imprimir encabezados
    cout << BG_BLUE << WHITE;
    for (const auto& col : colNames) {
        cout << setw(15) << left << col.substr(0, 15);
    }
    cout << RESET << endl;
    
    // Imprimir datos
    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        count++;
        for (int i = 0; i < colCount; i++) {
            const char* val = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            if (val) {
                cout << setw(15) << left << val;
            } else {
                cout << setw(15) << left << "NULL";
            }
        }
        cout << endl;
    }
    
    if (count == 0) {
        cout << BG_YELLOW << WHITE << " No se encontraron resultados. " << RESET << endl;
    } else {
        cout << GREEN << "\nTotal de registros encontrados: " << count << RESET << endl;
    }
    
    sqlite3_finalize(stmt);
    
    cout << endl << CYAN << "Presione Enter para continuar..." << RESET;
    cin.ignore();
}

// Función para importar datos desde CSV
void importData() {
    showHeader("IMPORTAR DATOS DESDE CSV");
    
    string filename;
    cout << CYAN << "Nombre del archivo CSV: " << RESET;
    getline(cin, filename);
    
    ifstream file(filename);
    if (!file.is_open()) {
        cout << BG_RED << WHITE << " Error al abrir archivo! " << RESET << endl;
        Sleep(2000);
        return;
    }
    
    string tableName;
    cout << CYAN << "Nombre de la tabla destino: " << RESET;
    getline(cin, tableName);
    
    if (!tableExists(tableName)) {
        cout << BG_RED << WHITE << " Error: La tabla no existe! " << RESET << endl;
        Sleep(2000);
        return;
    }
    
    // Leer encabezados
    string line;
    vector<string> headers;
    if (getline(file, line)) {
        stringstream ss(line);
        string header;
        while (getline(ss, header, ',')) {
            // Eliminar comillas si existen
            if (header.front() == '"' && header.back() == '"') {
                header = header.substr(1, header.size() - 2);
            }
            headers.push_back(header);
        }
    }
    
    // Construir INSERT
    string sql = "INSERT INTO \"" + tableName + "\" (";
    for (size_t i = 0; i < headers.size(); i++) {
        sql += "\"" + headers[i] + "\"";
        if (i < headers.size() - 1) sql += ", ";
    }
    sql += ") VALUES (";
    for (size_t i = 0; i < headers.size(); i++) {
        sql += "?";
        if (i < headers.size() - 1) sql += ", ";
    }
    sql += ");";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cout << BG_RED << WHITE << " Error al preparar INSERT: " << sqlite3_errmsg(db) << RESET << endl;
        Sleep(3000);
        return;
    }
    
    // Leer y procesar datos
    int count = 0;
    while (getline(file, line)) {
        stringstream ss(line);
        string value;
        int col = 0;
        
        while (getline(ss, value, ',')) {
            // Eliminar comillas si existen
            if (value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.size() - 2);
            }
            
            if (col < static_cast<int>(headers.size())) {
                sqlite3_bind_text(stmt, col+1, value.c_str(), -1, SQLITE_TRANSIENT);
            }
            col++;
        }
        
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            cout << BG_RED << WHITE << " Error al insertar fila: " << sqlite3_errmsg(db) << RESET << endl;
        } else {
            count++;
        }
        
        sqlite3_reset(stmt);
    }
    
    sqlite3_finalize(stmt);
    file.close();
    
    cout << BG_GREEN << WHITE << " " << count << " registros importados exitosamente! " << RESET << endl;
    Sleep(2000);
}

// Función principal
int main() {
    // Habilitar secuencias de escape ANSI en Windows
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
    
    setConsoleTitle("SQLite CRUD Manager");
    
    // Abrir base de datos
    if (sqlite3_open("basedatos.db", &db) != SQLITE_OK) {
        cout << BG_RED << WHITE << " Error al abrir base de datos: " << sqlite3_errmsg(db) << RESET << endl;
        return 1;
    }

    // Configurar SQLite para mejor rendimiento
    sqlite3_exec(db, "PRAGMA synchronous = OFF;", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "PRAGMA journal_mode = MEMORY;", nullptr, nullptr, nullptr);

    // Menú principal
    int choice;
    while (true) {
        clearScreen();
        drawLine(80, '=', BOLD + BG_BLUE + WHITE);
        cout << BOLD + BG_BLUE + WHITE << "  MENU PRINCIPAL - SQLite CRUD Manager " << RESET << endl;
        drawLine(80, '=', BOLD + BG_BLUE + WHITE);
        
        cout << BOLD << "\n " << BG_GREEN << WHITE << "1. " << RESET << BOLD << " Crear Tabla          " << RESET << endl;
        cout << BOLD << " " << BG_GREEN << WHITE << "2. " << RESET << BOLD << " Eliminar Tabla       " << RESET << endl;
        cout << BOLD << " " << BG_GREEN << WHITE << "3. " << RESET << BOLD << " Insertar Datos       " << RESET << endl;
        cout << BOLD << " " << BG_GREEN << WHITE << "4. " << RESET << BOLD << " Actualizar Datos     " << RESET << endl;
        cout << BOLD << " " << BG_GREEN << WHITE << "5. " << RESET << BOLD << " Listar Tablas        " << RESET << endl;
        cout << BOLD << " " << BG_GREEN << WHITE << "6. " << RESET << BOLD << " Ver Datos de Tabla   " << RESET << endl;
        cout << BOLD << " " << BG_GREEN << WHITE << "7. " << RESET << BOLD << " Buscar Datos         " << RESET << endl;
        cout << BOLD << " " << BG_GREEN << WHITE << "8. " << RESET << BOLD << " Importar desde CSV   " << RESET << endl;
        cout << BOLD << " " << BG_RED << WHITE << "9. " << RESET << BOLD << " Salir               " << RESET << endl;
        
        drawLine(80, '-', BOLD + CYAN);
        cout << BOLD << CYAN << " Seleccion: " << RESET;
        
        string input;
        getline(cin, input);
        
        try {
            choice = stoi(input);
        } catch (...) {
            choice = 0;
        }
        
        switch (choice) {
            case 1: createTable(); break;
            case 2: deleteTable(); break;
            case 3: insertData(); break;
            case 4: updateData(); break;
            case 5: listTables(); break;
            case 6: viewTableData(); break;
            case 7: searchData(); break;
            case 8: importData(); break;
            case 9: 
                sqlite3_close(db); 
                clearScreen();
                cout << BOLD + BG_BLUE + WHITE;
                drawLine(80, '=', BOLD + BG_BLUE + WHITE);
                cout << "  Gracias por usar SQLite CRUD Manager! " << endl;
                drawLine(80, '=', BOLD + BG_BLUE + WHITE);
                cout << RESET << endl;
                return 0;
            default:
                cout << BG_RED << WHITE << " Opcion invalida! Intente nuevamente. " << RESET << endl;
                Sleep(1500);
        }
    }
}


