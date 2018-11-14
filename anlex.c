/*
 *	Analizador Lexico
 *	Curso: Compiladores y Lenguajes de Bajo de Nivel
 *	Practica de Programacion Nro. 1
 *
 *	Descripcion:
 *	Implementa un analizador lexico que reconoce numeros, identificadores,
 * 	palabras reservadas, operadores y signos de puntuacion para un lenguaje
 * 	con sintaxis tipo JSON.
 *
 */

/*********** Inclusion de cabecera **************/
#include "anlex.h"
/************* Variables globales **************/

int consumir;			/* 1 indica al analizador lexico que debe devolver
						el sgte componente lexico, 0 debe devolver el actual */

char cad[5*TAMLEX];		// string utilizado para cargar mensajes de error
token t;				// token global para recibir componentes del Analizador Lexico

// variables para el analizador lexico
FILE *archivo;			// Fuente json
FILE *archivoSalida;	// Archivo de salida
char buff[2*TAMBUFF];	// Buffer para lectura de archivo fuente
char id[TAMLEX];		// Utilizado por el analizador lexico
int delantero=-1;		// Utilizado por el analizador lexico
int fin=0;				// Utilizado por el analizador lexico
int numLinea=1;			// Numero de Linea

/**************** Funciones **********************/
// Rutinas del analizador lexico
void error(const char* mensaje)
{
	printf("Lin %d: Error Lexico. %s.\n",numLinea,mensaje);
}

void asignarArchivo(FILE *f) {
	archivo = f;
}

void sigLex()
{
	int i=0;
	char c=0;
	int acepto=0;
	int estado=0;
	char msg[41];
	int isspecial;
	entrada e;

	while((c=fgetc(archivo))!=EOF)
	{

		if (c==' '){
			fprintf(archivoSalida," ");//se escribe en el archivo la cantidad de
			//espacios en blanco con el que cuenta
			continue;	//eliminar espacios en blanco
		}else if( c=='\t'){
			fprintf(archivoSalida,"\t");//se escribe en el archivo la cantidad de
			// tab con el que cuenta
			continue;//elimina el tab para el lexer
		}
		else if(c=='\n'){
			fprintf(archivoSalida,"\n");//se escribe en el archivo la cantidad de
			//enter con el que cuenta
			numLinea++;//incrementar el numero de linea
			continue;//se elimina el enter
		}
		else if (isalpha(c))
		{
			//es un identificador (o palabra reservada)
			i=0;
			do{
				id[i]=c;
				i++;
				c=fgetc(archivo);
				if (i>=TAMLEX)
					error("Longitud de Identificador excede tamanho de buffer");
			}while(isalpha(c));
			id[i]='\0';
			if (c!=EOF)
				ungetc(c,archivo);
			else
				c=0;
				t.pe=buscar(id);
				t.compLex=t.pe->compLex;
				break;
		}

		else if (isdigit(c))
		{
				//es un numero
				i=0;
				estado=0;
				acepto=0;
				id[i]=c;

				while(!acepto)
				{
					switch(estado){
					case 0: //una secuencia netamente de digitos, puede ocurrir . o e
						c=fgetc(archivo);
						if (isdigit(c))
						{
							id[++i]=c;
							estado=0;
						}
						else if(c=='.'){
							id[++i]=c;
							estado=1;
						}
						else if(tolower(c)=='e'){
							id[++i]=c;
							estado=3;
						}
						else{
							estado=6;
						}
						break;

					case 1://un punto, debe seguir un digito (caso especial de array, puede venir otro punto)
						c=fgetc(archivo);
						if (isdigit(c))
						{
							id[++i]=c;
							estado=2;
						}
						else if(c=='.')
						{
							i--;
							fseek(archivo,-1,SEEK_CUR);
							estado=6;
						}
						else{
							sprintf(msg,"No se esperaba '%c'",c);
							estado=-1;
						}
						break;
					case 2://la fraccion decimal, pueden seguir los digitos o e
						c=fgetc(archivo);
						if (isdigit(c))
						{
							id[++i]=c;
							estado=2;
						}
						else if(tolower(c)=='e')
						{
							id[++i]=c;
							estado=3;
						}
						else
							estado=6;
						break;
					case 3://una e, puede seguir +, - o una secuencia de digitos
						c=fgetc(archivo);
						if (c=='+' || c=='-')
						{
							id[++i]=c;
							estado=4;
						}
						else if(isdigit(c))
						{
							id[++i]=c;
							estado=5;
						}
						else{
							sprintf(msg,"No se esperaba '%c'",c);
							estado=-1;
						}
						break;
					case 4://necesariamente debe venir por lo menos un digito
						c=fgetc(archivo);
						if (isdigit(c))
						{
							id[++i]=c;
							estado=5;
						}
						else{
							sprintf(msg,"No se esperaba '%c'",c);
							estado=-1;
						}
						break;
					case 5://una secuencia de digitos correspondiente al exponente
						c=fgetc(archivo);
						if (isdigit(c))
						{
							id[++i]=c;
							estado=5;
						}
						else{
							estado=6;
						}break;
					case 6://estado de aceptacion, devolver el caracter correspondiente a otro componente lexico
						if (c!=EOF)
							ungetc(c,archivo);
						else
							c=0;
						id[++i]='\0';
						acepto=1;
						t.pe=buscar(id);
						if (t.pe->compLex==-1)
						{
							strcpy(e.lexema,id);
							e.compLex=LIT_NUMERICO;
							insertar(e);
							t.pe=buscar(id);
						}
						t.compLex=LIT_NUMERICO;
						break;
					case -1:
						if (c==EOF)
							error("No se esperaba el fin de archivo");
						else
							error(msg);
						exit(1);
					}
				}
			break;
		}
		else if (c==':')
		{
            t.compLex=DOS_PUNTOS;
            t.pe=buscar(":");
			break;
		}
		else if (c==',')
		{
			t.compLex=COMA;
			t.pe=buscar(",");
			break;
		}
		else if (c=='[')
		{
			t.compLex=L_CORCHETE;
			t.pe=buscar("[");
			break;
		}
		else if (c==']')
		{
			t.compLex=R_CORCHETE;
			t.pe=buscar("]");
			break;
		}
		else if (c=='\"')
		{//cadena de caracteres
			i=0;
			id[i]=c;
			i++;
			do{
				c=fgetc(archivo);
				if (c=='\"')
				{
					c=fgetc(archivo);
					if (c=='\"')
					{
						id[i]=c;
						i++;
						id[i]=c;
						i++;
					}
					else
					{
						id[i]='\"';
						i++;
						break;
					}
				}
				else if(c==EOF)
				{
					error("Se llego al fin de archivo sin finalizar un literal");
				}
				else{
					id[i]=c;
					i++;
				}
				isspecial=(int)c;/*variable para saber si el caracter es especial,como
				por ejemplo el acento en el fuente de JSON, si lo es retorna negativo*/
			}while(isascii(c) || isspecial < 0);
			id[i]='\0';
			if (c!=EOF)
				ungetc(c,archivo);
			else
				c=0;
			t.pe=buscar(id);
			t.compLex=t.pe->compLex;
			if (t.pe->compLex==-1)
			{
				strcpy(e.lexema,id);
                e.compLex=LIT_CADENA;
				insertar(e);
				t.pe=buscar(id);
				t.compLex=e.compLex;
			}
			break;
		}
		else if (c=='{')
		{
			t.compLex=L_LLAVE;
			t.pe=buscar("{");
			break;
		}
		else if (c=='}')
		{
			t.compLex=R_LLAVE;
			t.pe=buscar("}");
			break;
		}
		else if (c!=EOF)
		{
			sprintf(msg,"%c no esperado",c);
			error(msg);
		}
	}
	if (c == EOF)
	{
		t.compLex = EOF;
		//strcpy(e.lexema,"EOF");
		sprintf(e.lexema,"EOF");
		t.pe=&e;
	}

}
int escribirArchivo(int compLex){
	switch (compLex) {
		case L_CORCHETE:
			fprintf(archivoSalida,"%s ","L_CORCHETE");
			break;
		case R_CORCHETE:
			fprintf(archivoSalida,"%s ","R_CORCHETE");
			break;
		case L_LLAVE:
			fprintf(archivoSalida,"%s ","L_LLAVE");
			break;
		case R_LLAVE:
			fprintf(archivoSalida,"%s ","R_LLAVE");
			break;
		case COMA:
			fprintf(archivoSalida,"%s ","COMA");
			break;
		case DOS_PUNTOS:
			fprintf(archivoSalida,"%s ","DOS_PUNTOS");
			break;
		case LIT_CADENA:
			fprintf(archivoSalida,"%s ","STRING");
			break;
		case LIT_NUMERICO:
			fprintf(archivoSalida,"%s ","NUMERO");
			break;
		case PR_TRUE:
			fprintf(archivoSalida,"%s ","PR_TRUE");
			break;
		case PR_FALSE:
			fprintf(archivoSalida,"%s ","PR_FALSE");
			break;
		case PR_NULL:
			fprintf(archivoSalida,"%s ","PR_NULL");
			break;
		case -1:
			break;
	}
}

int linea(){
    return numLinea;
}

/*
int main(int argc,char* args[])
{
	initTabla();
	initTablaSimbolos();

	if(argc > 1)
	{
		if (!(archivo=fopen(args[1],"rt")))
		{
			printf("Archivo no encontrado.\n");
			exit(1);
		}
		if(!(archivoSalida = fopen("output.txt","w"))){
			printf("\n ocurrio un error al generar el archivo por favor intentalo de nuevo");
			exit(1);
		}
		while (t.compLex!=EOF){
			sigLex();
			escribirArchivo(t.compLex);
		}
		fclose(archivo);
	}else{
		printf("Debe pasar como parametro el path al archivo fuente.\n");
		exit(1);
	}
	return 0;
}
*/
