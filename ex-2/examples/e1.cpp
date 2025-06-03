#include "stdio.h"
#include "string.h"
#include "stdlib.h"
//----------------------Global Declarition---------------------------------
#define SIZE 20
#define sSIZE 12         //There are sSIZE status
#define aSIZE 6          //There will ecounter aSIZE symbol
#define gSIZE 3          //May be goto next gSIZE status
#define geSIZE 6         //There are geSIZE generate expression
#define MAXSIZE 3

//---------------------Finish defining struct-------------------------------------
typedef struct Ge
{
        char head;       //Leftpart of Generate Expression
        char gen[4];       //Rightpart of Generate Expression 
}Generate;//--------------------------------Generate Expression base datastruct
typedef struct A
{
 int st[aSIZE];       //aSIZE status when encountering terminated symbol
 int re[aSIZE];       //Using reduce 
}Action;//----------------------------------Action table base datastruct
typedef struct G
{
 char head[gSIZE];      //Nonterminated symbol :'E' 'F' 'T'..etc
 int gt[gSIZE];       //Mark the next status
}GOTO;//------------------------------------GOTO table base datastruct
int status[SIZE];                          //stack of status                                 
int  sta_Index;           //top of stack of status  
char symbol[SIZE];                          //stack of symbol
int  sym_Index;        //Current index of symbol stack
char expression[SIZE];                      //Inputed expression
int  exp_Index;                             //index of inputed expression
int  exp_top;        //top of expression that inputed
int  step;         //accumulated steps
int  IsAccept = 0;       //Initlize accept flag to 0
Generate gene[geSIZE +1];
Action act[sSIZE];
GOTO go[sSIZE];
//------------------------------------------------------------------------
void GOTOTable(int sta, char symb);
/*******************************************
*@Name:        Syntax
*@Description: Printing Syntax
*@Param:       void
*@Return:      void
********************************************/
void Syntax(void)
{
 printf("-------LR(1) Analysis Program -------- \n");
 printf("*@Author:    oDon 200490513101  WJ-0411\n");
 printf("*@Create: 2006-12-7\n");
 printf("*@Copyright: yuanonline@hotmail.com\n");
 printf("*@Corporation: xxxx.CO.TD\n");
 printf("*@Description: LR(1) Syntax Analysister\n");
 printf("--------------------------------------\n\n");
 printf("-------------LR(1) Syntax---------------\n");
 printf(" (0)E -> E + T\n (1)E -> T\n (2)T -> T*F\n (3)T -> F\n (4)F -> E\n (5)F -> (i)\n");
 printf("-----------------------------------------\n");
}
/*******************************************
*@Name:        InputExpression
*@Description: Input analysis Expression
      Exiting when it is illegal
*@Param:       void
*@Return:      void
********************************************/
void InputExpression(void)
{
     char ch;
     printf("请输入分析串");
     printf("[包括:{+ - * /（ ）i #}以'#'结束]:\n");
     exp_Index = 0;
  do                                
  {
     scanf("%c",&ch);
     if ((ch!='i') &&(ch!='+') &&(ch!='*')&&(ch!='#')&&(ch!='(')&&(ch!=')'))
     {
     printf("Illegal Word inside...Press any key to EXIT!\n");
     getchar();
     exit(0);
     }
     expression[exp_Index++]=ch;
  }while(ch!='#');                       
  printf("---------Valid Analysis String---------\n");
  getchar();
}
/*******************************************
*@Name:        PrintExpression
*@Description: Output analysis Expression for test
*@Param:       void
*@Return:      void
********************************************/
void PrintExpression(void)
{
     int i = 0;
     printf("You have inputed below:\n");
     for(i = 0; i < exp_Index; i++)
     printf("%c",expression[i]);
  printf("\n");
}
/*******************************************
*@Name:        PrintStatus
*@Description: Output status stack
*@Param:       void
*@Return:      void
********************************************/
void PrintStatus(void)
{
 int i = 0;
 for(i = 0; i <= sta_Index; i++)
 {
  printf("%d", status[i]);
 }
 printf("\t\t");
}
/*******************************************
*@Name:        PrintRestExp
*@Description: Output the rest of symbol
*@Param:       void
*@Return:      void
********************************************/
void PrintRestExp(void)
{
 int i = 0;
 for(i = 0; i < exp_top; i++)
  printf(" ");
 for(i = exp_top; i <= exp_Index; i++)
 {
  printf("%c", expression[i]);
 }
 printf("\t\t");
}
/*******************************************
*@name:        Format
*@Description: output format
*@Param:       void
*@Return:      void
********************************************/
void Format()
{
 printf("步骤 \t 状态栈 \t符号栈   \t 剩余输入串 \t 动作\n");
}
/*******************************************
*@Name:        InitTable
*@Description: Initlize LR(1) syntax table,stack etc.
*@Param:       void
*@Return:      void
********************************************/
void Initlize(void)
{
//-------------------------------------Initlize generate expression
 gene[1].head = 'E';
 strcpy(gene[1].gen,"E+T");
 gene[2].head = 'E';
 strcpy(gene[2].gen,"T");
 
 gene[3].head = 'T';
 strcpy(gene[3].gen,"T*F");
 
 gene[4].head = 'T';
 strcpy(gene[4].gen,"F");
 
 gene[5].head = 'F';
 strcpy(gene[5].gen,"(E)");
 
 gene[6].head = 'F';
 strcpy(gene[6].gen,"i");
 //-----------------------------------Finish initlizing generate expression
 //-----------------------------------Initlize Action table
 act[0].st[0] = 5;
 act[0].st[3] = 4;
 
 act[1].st[1] = 6;
// act[1].st[5] = 10;
 act[2].re[1] = 2;
 act[2].st[2] = 7;
 act[2].re[4] = 2;
 act[2].re[5] = 2;
 
 act[3].re[1] = 4;
 act[3].re[2] = 4;
 act[3].re[4] = 4;
 act[3].re[5] = 4;
 
 act[4].st[1] = 5;
 act[4].st[3] = 4;
 
 act[5].re[1] = 6;
 act[5].re[2] = 6;
 act[5].re[4] = 6;
 act[5].re[5] = 6;
 
 act[6].st[0] = 5;
 act[6].st[3] = 4;
 
 act[7].st[0] = 5;
 act[7].st[3] = 4;
 
 act[8].re[1] = 6;
 act[8].re[4] = 11;
 
 act[9].re[1] = 1;
 act[9].re[2] = 7;
 act[9].re[4] = 1;
 act[9].re[5] = 1;
 
 act[10].re[1] = 3;
 act[10].re[2] = 3;
 act[10].re[4] = 3;
 act[10].re[5] = 3;
 
 act[11].re[1] = 5;
 act[11].re[2] = 5;
 act[11].re[4] = 5;
 act[11].re[5] = 5;
 //-----------------------------------Finish initlizing Action table
 //-----------------------------------Initlize Goto table
 go[0].head[0] = 'E';
 go[0].head[1] = 'T';
 go[0].head[2] = 'F';
 go[0].gt[0] = 1;
 go[0].gt[1] = 2;
 go[0].gt[2] = 3;
 
 go[4].head[0] = 'E';
 go[4].head[1] = 'T';
 go[4].head[2] = 'F';
 go[4].gt[0] = 8;
 go[4].gt[1] = 2;
 go[4].gt[2] = 3;
 
 go[6].head[1] = 'T';
 go[6].head[2] = 'F';
 go[6].gt[1] = 9;
 go[6].gt[2] = 3;
 
 go[7].head[2] = 'F';
 go[7].gt[2] = 10;
 //-----------------------------------Finish initlizing Goto
 //-----------------------------------Initlize global vari
 sta_Index = 0;                       
 status[sta_Index] = 0;                
 step = 1;
 sym_Index = 0;
 symbol[sym_Index] = '#';
 IsAccept = 0;
 exp_top = 0;
}
/*******************************************
*@Name:        Reduce
*@Description: Using Generate expression to reduce symbol
*@Param:       int sta, char symb,int col
*@Return:      void
********************************************/
void Reduce(int sta, char symb,int col)
{
 int i = 0;
 for(i = 0; i < strlen(gene[act[sta].re[col]].gen); i++)
 {
  symbol[sym_Index--] = '\0';
 }
 symbol[++sym_Index] = gene[act[sta].re[col]].head;
 for(i = 0; i < strlen(gene[act[sta].re[col]].gen) ; i++)
 {
  status[sta_Index - i] = '\0';
 }
 sta_Index -= i; 
 GOTOTable(status[sta_Index], symbol[sym_Index]);
}
/*******************************************
*@name:        ActionTable
*@Description: Into Action table
*@Param:       int sta, char symb,int col
*@Return:      void
********************************************/
void ActionTable(int sta, char symb,int col)
{
 if(sta == 1 && col == 5)
 {
  printf(">Accept<\n");
  IsAccept = 1;
  return;
 }
 if(act[sta].st[col] != 0)
 {
  printf("Action\n");
  status[++sta_Index] = act[sta].st[col];
  symbol[++sym_Index] = symb;
  exp_top ++;
 }
 else if(act[sta].re[col] != 0)
 {
  printf("Reduce\n");
  Reduce(sta, symb, col);
 }
 else
 {
  printf(">error<\n");
  getchar();
  exit(1);
 }
  
}
/*******************************************
*@name:        GOTOTable
*@Description: Goto
*@Param:       int sta, char symb
*@Return:      void
********************************************/
void GOTOTable(int sta, char symb)
{
 int i = 0;
 for(i = 0; i < sSIZE; i++)
 {
   if(go[sta].head[i] == symb)
  {
   //printf("Head %d\n",go[sta].gt[i]);
   status[++sta_Index] = go[sta].gt[i];
   return;
  }
 }
}
/*******************************************
*@name:        Launch
*@Description: Lunch program
*@Param:       void
*@Return:      void
********************************************/
void Launch(void)
{
 int s = status[sta_Index];
 char exp = expression[exp_top];
 char sym = symbol[sym_Index];
 while(IsAccept != 1)
 {
  s = status[sta_Index];
  exp = expression[exp_top];
  sym = symbol[sym_Index];
  printf("%d\t ",step++);
  PrintStatus();
  printf(" %s\t\t", symbol);
  PrintRestExp();
  switch(exp)
  {
   case 'i':
    ActionTable(s, exp, 0); 
    break;
   case '+':
    ActionTable(s, exp, 1);
    break;
   case '*':
    ActionTable(s, exp, 2);
    break;
   case '(':
    ActionTable(s, exp, 3);
    break;
   case ')':
    ActionTable(s, exp, 4);
    break;
   case '#':
    ActionTable(s, exp, 5);
    break;
  }
 }
}

//-----------------------------------------
//-----------------MAIN--------------------
//-----------------------------------------
int main()
{
  Syntax();
  InputExpression();
  //PrintExpression();
  Initlize();
  Format();
  Launch();
  getchar();
  return 1;
}
