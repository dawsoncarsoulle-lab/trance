%{
#include <assert.h>
/* Les codes tokens commencent à 258 car 0-257 sont réservés par ASCII/Flex */
#define TOK_IF 258
#define TOK_THEN 259
/* Ajoutez les autres define ici pour l'exercice 1 (TOK_ELSE, TOK_READ, etc.) */
%}

%%

/* Règles d'analyse */

if {
    assert(printf("'if' found\n"));
    return TOK_IF;
}

then {
    assert(printf("'then' found\n"));
    return TOK_THEN;
}

/* Règle par défaut pour afficher les caractères non reconnus */
. {
    printf("%s", yytext);
}

%%
/* Code C supplémentaire si nécessaire */
