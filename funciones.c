

#include <stdio.h>
#include <stdlib.h>

int len(char* s1) {
    int i = 0;
    while (s1[i] != 0) {
        i++;
    }
    return i;
}

// char* strDuplicate(char* src): Duplica un string en memoria. Debe contar la cantidad de caracteres de src,
// solicitar mediante malloc la memoria necesaria (incluyendo el
// terminador ’\0’), y copiar todos los caracteres. Retorna el puntero al nuevo string.

char* strDuplicate(char* src){
    int cantLetras = len(src); // -> 9 letras (no inclue \0)
    char *srcDuplicado = (char*) malloc ((cantLetras + 1) * sizeof(char)); // -> pedimos la memoria para duplicar
    for (int i = 0; i < cantLetras; i++){
        srcDuplicado[i] = src[i];
    }
    srcDuplicado[cantLetras] = "\0";
    return srcDuplicado;
}

int strCompare(char* s1, char* s2){
    int count = 0;
    while(s1[count] == s2[count]){
        count++;
    }
    if(s1[count] < s2[count]){
        return 1;
    }
    if(s1[count] > s2[count]){
        return -1;
    }
    return 0;
}
/* Compara dos strings lexicogr´aficamente. Devuelve 0 si son iguales, 1 si el primer string es menor al segundo, -1 en otro caso. Atenci´on:
es la convenci´on inversa a la de strcmp. Por ejemplo, strCompare("a", "b") devuelve
1, strCompare("b", "a") devuelve -1, y strCompare("casa", "casas") devuelve 1 (un
prefijo es menor).
*/


char* strConcatenate(char* src1, char* src2){
    int lenSrc1 = len(src1);
    int lenSrc2 = len(src2);
    char *concat = (char*) malloc(sizeof(char) * (lenSrc1 + lenSrc2 + 1));
    for (int i = 0; i < lenSrc1; i++) {
        concat[i] = src1[i];
    }
    for (int j = 0; j < lenSrc2; j++) {
        concat[lenSrc1 + j] = src2[j];
    }
    concat[lenSrc1 + lenSrc2] = 0;
    free(src1);
    free(src2);
    return concat;
}

// char* strConcatenate(char* src1, char* src2): Retorna un nuevo string din´amico
// que contiene una copia de todos los caracteres de src1 seguidos de src2. Adem´as, la memoria original apuntada por src1 y src2 debe ser liberada internamente utilizando free.
// Por lo tanto, ambos argumentos deben haber sido obtenidos con malloc (por ejemplo,
// mediante strDuplicate) y ser punteros distintos: pasar un literal como "abc", o el mismo
// puntero dos veces, produce un free inv´alido.
