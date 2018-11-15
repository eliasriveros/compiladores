/*********** Inclusión de cabecera **************/
#include "anlex.h"

/************* Variables globales **************/
token t;
FILE *archivo; // Fuente JSON
FILE *output;
char msg[41]; // Mensaje de error.
char trad[41];
short error_flag = 0; // Bandera para mensajes. 
short nivel=0;
/**************** Funciones **********************/
// Funciones para los no terminales
void json(void);
void element(void);
void array(void);
void ar(void);
void element_list(void);
void el(void);
void object(void);
void o(void);
void attribute_list(void);
void attribute(void);
void al(void);
void attribute_name(void);
void attribute_value(void);

// Funciones de traducción.
char* tabulador(char*);
void destabulador(char*);

// Función de mensaje.
void error_msg(char* mensaje) {
    error_flag = 1;
    printf("Lin %d: Error Sintáctico. %s.\n", linea(), mensaje);	
}

void getToken(void) {
	sigLex();
}

void match(int n) {
	if (t.compLex == n) getToken();
	else error_msg("Error en el match");
}

// Función que concatena tabulaciones.
char* tabulador(char* texto){
    char *tabs= malloc(sizeof(texto));
    for(short i=0;i<nivel;i++){
        strcat(tabs,"\t");
    }
    strcat(tabs,texto);
    return tabs;
}

// Remueve todas las tabulaciones.
void destabulador(char* texto){   
    char *pr = texto, *pw = texto;
    while (*pr) {
        *pw = *pr++;
        pw += (*pw != '\t');
    }
    *pw = '\0';
}

void json() {
	element();
	match(EOF);
}

void element() {
	if (t.compLex == L_LLAVE) {
		object();
	} else if (t.compLex == L_CORCHETE) {
		array();
	} else if (t.compLex == R_CORCHETE || t.compLex == R_LLAVE || t.compLex == COMA) {
		sprintf(msg, "Se esperaba un \"{\" o \"[\" no \"%s\"", t.pe->lexema);
        error_msg(msg);
	} else getToken();
}

void array() {
    if (t.compLex == L_CORCHETE) {
        match(L_CORCHETE);
        ar();  
        nivel++;
        strcpy(trad,tabulador(trad));
        nivel--;
    } else if (t.compLex == R_CORCHETE || t.compLex == R_LLAVE || t.compLex == COMA) {
        sprintf(msg, "Se esperaba un \"[\" no \"%s\"", t.pe->lexema);
        error_msg(msg);
    } else getToken();
}

void ar() {
    if (t.compLex == R_CORCHETE) {
        match(R_CORCHETE); 
    } else if (t.compLex == L_CORCHETE || t.compLex == L_LLAVE) {
        element_list();
        match(R_CORCHETE);
    } else if (t.compLex == R_LLAVE) {
        sprintf(msg,"Se esperaba un \"[\" o \"]\" o \"{\" no \"%s\"", t.pe->lexema);
        error_msg(msg);
    }  else getToken();
}

void element_list() {
    if (t.compLex == L_CORCHETE || t.compLex == L_LLAVE) {
        element();
        el();    
    } else if (t.compLex == R_CORCHETE) {
        sprintf(msg,"Se esperaba un \"[\" o \"{\" no \"%s\"", t.pe->lexema);
        error_msg(msg);
    } else getToken();   
}

void el() {
    if (t.compLex == COMA) {
        match(COMA);
        nivel++; // Aumenta el nivel de tabulación ante un elemento.
        element();
        el();
    } else if (t.compLex != R_CORCHETE) { 
        getToken(); 
    }
}

void object() {
    if (t.compLex == L_LLAVE) {
        match(L_LLAVE);
        o(); 
        nivel--; // Cuando el object termina se baja un nivel.
    } else if (t.compLex == R_CORCHETE || t.compLex == R_LLAVE || t.compLex == COMA) {
        sprintf(msg,"Se esperaba un \"{\" no \"%s\"", t.pe->lexema);
        error_msg(msg);
    } else getToken();
}

void o() {
    if (t.compLex == R_LLAVE) {
        match(R_LLAVE);   
    } else if (t.compLex == LIT_CADENA) {
        attribute_list();
        match(R_LLAVE);
    } else if (t.compLex == R_CORCHETE || t.compLex == COMA) {
        sprintf(msg,"Se esperaba un \"}\" o \"string\" no \"%s\"", t.pe->lexema);
        error_msg(msg);
    } else getToken();     
}

void attribute_list() {
    if (t.compLex == LIT_CADENA) {
        attribute();
        al(); 
    } else if (t.compLex == R_LLAVE) {
        sprintf(msg,"Se esperaba un \"string\" no \"%s\"", t.pe->lexema);
        error_msg(msg);
    } else getToken(); 
}


void attribute() {
    if(t.compLex == LIT_CADENA) {
        // Puntero auxiliar para extraer las commillas.
        char* aux=t.pe->lexema;
        aux++;
        aux[strlen(aux)-1] = '\0';
        
        sprintf(trad,"<%s>",aux);
        strcpy(trad,tabulador(trad)); // Tabulador devuelve las tabulaciones según el nivel
        fputs(trad,output); // Se escribe en el archivo;
        attribute_name();
        match(DOS_PUNTOS);
        attribute_value(); 
        destabulador(trad); // Se remueve todas las tabulaciones.
        sprintf(trad,"</%s>\n", aux);
        fputs(trad,output); // Se escribe en el archivo;
    } else if(t.compLex == R_LLAVE || t.compLex == COMA) {
        sprintf(msg,"Se esperaba un \"string\" no \"%s\"", t.pe->lexema);
        error_msg(msg);
    } else getToken();    
}

void al() {
    if (t.compLex == COMA) {
        match(COMA);
        attribute();
        al();
    } else if (t.compLex == R_LLAVE) { 
        getToken(); 
    }       
}

void attribute_name() {
    if (t.compLex == LIT_CADENA) {
        match(LIT_CADENA);
    } else getToken();
}

void attribute_value() {
    if (t.compLex == L_LLAVE || t.compLex == L_CORCHETE) {
        fputs("\n",output);
        nivel++; // Aumenta de nivel por elemento en el archivo fuente.
        element();
    } else if (t.compLex == LIT_CADENA) {
        fputs(t.pe->lexema,output);
        match(LIT_CADENA);
    } else if (t.compLex == LIT_NUMERICO) {
        fputs(t.pe->lexema,output);
        match(LIT_NUMERICO);
    } else if (t.compLex == PR_TRUE) {
        fputs(t.pe->lexema,output);
        match(PR_TRUE);
    } else if (t.compLex == PR_FALSE) {
        fputs(t.pe->lexema,output);
        match(PR_FALSE);
    } else if (t.compLex == PR_NULL) {
        fputs(t.pe->lexema,output);
        match(PR_NULL);
    } else if (t.compLex == R_LLAVE || t.compLex == COMA ||t.compLex == DOS_PUNTOS) {
        sprintf(msg,"Se esperaba un \"{\" o \"[\" o \"string\" o \"number\" o \"true\" o \"false\" o \"null\" no \"%s\"", t.pe->lexema);
        error_msg(msg);
    } else getToken(); 
}

void inicio(void) {
    json(); 
    if (t.compLex != EOF) error_msg("No se esperaba fin del archivo.");
}

void parser() {
    while (t.compLex != EOF) {
        getToken();
        inicio();
    }   
}

int main(int argc, char* args[]) {
    initTabla();
    initTablaSimbolos();
  	if ( argc > 1 ) {
		if (!(archivo=fopen(args[1], "rt"))) {
	        printf("Archivo no encontrado.\n");
	        exit(1);
		}

		while (t.compLex != EOF) {
            output = fopen( "output.txt", "w" );
            parser();
            if (error_flag == 0)
                printf("Sintácticamente correcto.\n");
            fclose(output);
		}
		fclose(archivo);
    } else {
		printf("Debe pasar como parametro el path al archivo fuente.\n");
		exit(1);
    }

    return 0;
}
