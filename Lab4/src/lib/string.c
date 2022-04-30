#include "string.h"

char* memory_counter = (char*)0x1000000;

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

int hex2int(char *hexStr)
{
    int value = 0;
    int tmpValue;
    while (*hexStr != '\0') {
        if (*hexStr >= 65 && *hexStr <= 70) {
            tmpValue = *hexStr - 55;
        } else if (*hexStr >= 97 && *hexStr <= 102) {
            tmpValue = *hexStr - 87;
        } else {
            tmpValue = *hexStr - '0';
        }

        value = (value << 4) + tmpValue;
        hexStr++;
    }

    return value;
}

void itohexa(int x, char str[], int d)
{
    int i = 0; 
    while (x) { 
        if((x%16) < 10)
            str[i++] = (x % 16) + '0'; 
        else
        {
            switch(x%16)
            {
                case 10:
                    str[i++] = 'a';
                    break;
                case 11:
                    str[i++] = 'b';
                    break;
                case 12:
                    str[i++] = 'c';
                    break;
                case 13:
                    str[i++] = 'd';
                    break;
                case 14:
                    str[i++] = 'e';
                    break;
                case 15:
                    str[i++] = 'f';
                    break;
            }
        }
        x = x / 16; 
    } 
  
    // If number of digits required is more, then 
    // add 0s at the beginning 
    while (i < d) 
        str[i++] = '0';
    
    str[i++] = 'x';
    str[i++] = '0';
    
    str[i] = '\0'; 
    reverse(str);
}

void* simple_malloc(unsigned int size)
{
    char* return_addr = memory_counter;
    memory_counter += size;

    return (void*)return_addr;
}

int strstr(char src[], char target[])
{
    for(int i=0 ; i<strlen(src) ; i++)
    {
        if( src[i] == target[0] )
        {
            int j;
            for( j=1 ; j<strlen(target) ; j++)
            {
                if(  src[i+j] != target[j])
                    break;
            }
            if( j==strlen(target) )
                return 1;
        }
    }
    return 0;
}

int atoi(char str[])
{
    int len = strlen(str);
    int res = 0;
    for(int i=0 ; i<len ; i++)
    {
        res *= 10;
        res += str[i]-'0';
    }
    return res;
}

char* strcpy(char src[], char des[])
{
    int i=0;
    while(src[i]!='\0')
    {
        des[i]=src[i];
        i++;
    }
    des[i]='\0';

    return des;
}