// mssql_test.c
// 
// For testing connection to Microsoft SQL Server via ODBC with Kerberos and 
// with username/password
//
// Example ~/.odbc.ini
// 
//   [MSSQLTest]
//   Driver = ODBC Driver 18 for SQL Server
//   Server = tcp:server.domain.no,1433
//   Encrypt = YES 
//   TrustServerCertificate = YES
//   Trusted_Connection = YES
//
// Bjorn Hofsvang bjorn(at)hofsvang.no  

#include <stdio.h>
#include <sql.h>
#include <sqlext.h>
#include <string.h>



int main(int argc, char *argv[]) 
{

    SQLHENV henv; // Environment handle
    SQLHDBC hdbc; // Connection handle
    SQLHSTMT hstmt; // Statement handle
    SQLRETURN ret; // Return code

    char szDSN[50] = "";
    char szUsername[50] = "";
    char szPassword[50] = "";


    
    // Get arguments
    if (argc < 2) {
        printf("Error no ODBC DSN specified\n\n");
        printf("Use %s DSN [username] [password]\n", argv[0]);
        return 1;
    } 
    strcpy(szDSN,argv[1]);


    if (argc > 2) {
        strcpy(szUsername,argv[2]);
    }

    if (argc > 3) {
        strcpy(szPassword,argv[3]);
    }

    // Allocate environment handle
    ret = SQLAllocHandle(SQL_HANDLE_ENV, 
                         SQL_NULL_HANDLE, 
                         &henv);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        printf("Error allocating environment handle\n");
        return 1;
    }

    // Set environment attributes
    ret = SQLSetEnvAttr(henv, 
                        SQL_ATTR_ODBC_VERSION, 
                        (SQLPOINTER)SQL_OV_ODBC3, 
                        0);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        printf("Error setting environment attributes\n");
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return 1;
    }

    // Allocate connection handle
    ret = SQLAllocHandle(SQL_HANDLE_DBC, 
                         henv, 
                         &hdbc);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        printf("Error allocating connection handle\n");
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return 1;
    }

    // Connect to the database
    ret = SQLConnect(hdbc, 
                     (SQLCHAR *)szDSN, // ODBC Dataset name 
                     SQL_NTS, 
                     (SQLCHAR *)szUsername, // Username 
                     SQL_NTS, 
                     (SQLCHAR *)szPassword, // Password , 
                     SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        printf("Error connecting to database\n");
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return 1;
    }

    // Allocate statement handle
    ret = SQLAllocHandle(SQL_HANDLE_STMT, 
                         hdbc, 
                         &hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        printf("Error allocating statement handle\n");
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return 1;
    }

    // Execute a simple query
    ret = SQLExecDirect(hstmt, 
                        (SQLCHAR *)"SELECT @@VERSION", 
                        SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        printf("Error executing query\n");
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return 1;
    }

    // Fetch and print results
    SQLCHAR column1[255]; // Assuming columns are VARCHAR, adjust size accordingly
    SQLLEN column1len; 
    while (SQLFetch(hstmt) == SQL_SUCCESS) {
        ret = SQLGetData(hstmt, 1, SQL_C_CHAR, column1, sizeof(column1), &column1len);
          if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
            printf("%s\n", column1);
        }
    }

    // Free resources 
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLDisconnect(hdbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);

    return 0;
}
