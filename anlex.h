/*********** Librerias utilizadas **************/
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<ctype.h>

/************* Definiciones ********************/
//a partir de 257
//Codigos

#define	L_CORCHETE		257
#define	R_CORCHETE		258
#define	L_LLAVE			259
#define	R_LLAVE			260
#define	COMA			261
#define	DOS_PUNTOS		262
#define	LIT_CADENA		263
#define	LIT_NUMERICO	264
#define	PR_TRUE			265
#define	PR_FALSE		266
#define	PR_NULL			267

// Fin Codigos
#define TAMBUFF 	5
#define TAMLEX 		50
#define TAMHASH 	101

/************* Estructuras ********************/

typedef struct entrada{
	//definir los campos de 1 entrada de la tabla de simbolos
	int compLex;
	char lexema[TAMLEX];
	struct entrada *tipoDato; // null puede representar variable no declarada
	// aqui irian mas atributos para funciones y procedimientos...

} entrada;

typedef struct {
	int compLex;
	entrada *pe;
} token;

/************* Prototipos ********************/
void insertar(entrada e);
entrada* buscar(const char *clave);
void initTabla();
void initTablaSimbolos();
void sigLex();
int escribirArchivo(int compLex);
int linea();
