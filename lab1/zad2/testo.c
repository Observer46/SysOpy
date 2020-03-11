#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(){
    char* txt1 =(char*) calloc(1,sizeof(char));
    //txt1[0] = '\0';
    char txt2[] = "boi\n";
    printf("%s",txt1);
    char* tmpString = (char*)calloc(strlen(txt1) + strlen(txt2),sizeof(char));
    sprintf(tmpString, "%s%s",txt1,txt2);
    printf("%s\n",tmpString);
    txt1 = tmpString;
    printf("\n\n\n\n");
    //sprintf(txt1,"%s%s",txt1,txt2);
    printf("%s",txt1);
    free(txt1);

    return 0;
}