#include "string.h"

void strset(char *string, int value, int size)
{
    for(int i=0 ; i<size ; i++)
        string[i] = value;
}

int strlen(char *string)
{
    int count = 0;
    while(1)
    {
        if( *(string+count) == '\0')
            break;
        count++;
    }
    
    return count;
}

int strequ(char *str1, char *str2)
{
    if(strlen(str1) != strlen(str2))
        return 0;
    else
    {
        for( int i=0 ; i<strlen(str1) ; i++)
        {
            if( str1[i] != str2[i] )
                return 0;
        }
    }
    
    return 1;
}

void reverse ( char * s )
{
    int i;
    char temp;

    for ( i = 0; i < strlen(s) / 2; i++ ) 
    {
        temp = s[strlen(s) - i - 1];
        s[strlen(s) - i - 1] = s[i];
        s[i] = temp;
    }
}

void itoa (int x, char str[], int d) 
{ 
    int i = 0; 
    while (x) { 
        str[i++] = (x % 10) + '0'; 
        x = x / 10; 
    } 
  
    // If number of digits required is more, then 
    // add 0s at the beginning 
    while (i < d) 
        str[i++] = '0'; 
    
    str[i] = '\0'; 
    reverse(str); 
}