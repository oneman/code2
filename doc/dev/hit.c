#include <stdio.h>
#include <unistd.h>
void tohex(int decimal_Number)
{
    int i = 1, j, temp;
    char hexa_Number[100];
    printf("%d ", decimal_Number);
    if (decimal_Number == 0) { printf("0 is 0 in hex\n"); return; }
    // if decimal number is not 
    // equal to zero then enter in
    // to the loop and execute the
    // statements
    while (decimal_Number != 0) {
        temp = decimal_Number % 16;
      
        // converting decimal number 
        // in to a hexa decimal
        // number
        if (temp < 10)
            temp = temp + 48;
        else
            temp = temp + 55 + 32;
        hexa_Number[i++] = temp;
        decimal_Number = decimal_Number / 16;
    }
    // printing the hexa decimal number
    printf("Hexadecimal value is: ");
    for (j = i - 1; j > 0; j--)
        printf("%c", hexa_Number[j]);
        
  printf("\n");
}

void tobase26(int decimal_Number)
{
    int i = 1, j, temp;
    char hexa_Number[100];
    printf("%d ", decimal_Number);
    // if decimal number is not 
    // equal to zero then enter in
    // to the loop and execute the
    // statements
    
    if (decimal_Number == 0) { printf("base26 value is: A\n"); return; }
    
    while (decimal_Number != 0) {
        temp = decimal_Number % 26;
        temp = temp + 65;
        hexa_Number[i++] = temp;
        decimal_Number = decimal_Number / 26;
    }
    // printing the hexa decimal number
    printf("base26 value is: ");
    for (j = i - 1; j > 0; j--)
        printf("%c", hexa_Number[j]);
        
  printf("\n");
}

int main(int argc, char **argv) {
  tohex(45);
  tobase26(45);
  tohex(26);
  tobase26(26);
  for (int i = 0; i < 4126; i++) {
    tohex(i);
    tobase26(i);
  }
  return 0;
}
