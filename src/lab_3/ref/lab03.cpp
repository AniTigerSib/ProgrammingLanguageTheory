#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ID_LEN 50
#define MAX_VARS 20

/*
     Программа приведена с упрощениями и сокращениями:
     - использован простейший контейнер для переменных - массив
     - жестко ограничена длина идентификатора
     - не допускаются пустые символы (разделители) между лексемами
*/

/*
Множество правил грамматики

  S ::= I=E;                  Оператор присваивания
  E ::= E+T | E-T | T         Операции + и -
  T ::= T*M | T/M | M         Операции * и /
  M ::= (E) | -M | I | C      Cкобки, унарный минус, переменная, константа
  I ::= A | AK                Идентификатор: первая буква,
  K ::= A | AK | D | DK          затем буквы и цифры
  C ::= D | CD                Константа: последовательность цифр
  A ::= a | b | ... | z | _   Буквы
  D ::= 0 | 1 | ... | 9       Цифры

Пустые символы (пробелы, ентеры и т.п.) внутри
оператора или выражения не допускаются, но могут
быть между операторами, перед первым оператором
и после последнего.

В файле lab03in.txt приведена правильная программа на языке,
порождаемом этой граммтикой. Конечно, ее легко можно (и даже
нужно) сделать неправльной в целях полноценного тестирования.
*/

// Обработка ошибок: вывод сообщения и завершение программы
void Error(const char * msg, const char * param);

// Обработка переменных:
long * VarPointer(const char * name);   // получение адреса переменной
long VarValue(const char * name);       // получение значения переменной
void VarPrint(void);                    // печать последней переменной
void VarPrintAll(void);                 // печать всех переменных

// Считать следующий символ
inline void Get(void);

// Процедуры разбора нетерминальных символов
void ProcS(void);
long ProcE(void);
long ProcT(void);
long ProcM(void);
long ProcI(void);   // там же внутри разбор A, K, D
long ProcC(void);   // D внутри, незачем для терминалов процедуры писать
long * ProcL(void); // это как I, только слева от =, потому и L, т.к.
                    // требуется присвоить значение этой переменной,
                    // а если переменная в выражении - то получить ее
                    // значение (это ProcI делает)


// Структура для хранения переменной (примитивно, насколько возможно)
struct var
{
  char name[MAX_ID_LEN];
  long value;
};

var v[MAX_VARS];          // массив переменных
int n = 0;                // их количество
int last = -1;            // и номер последней присвоенной

FILE * f = 0;
int c = EOF;



void Error(const char * msg, const char * param)
{
  printf("Error: ");        // вместо этой функции можно использовать
  printf(msg, param);       // собственный класс исключения, объект
  printf("\n");             // которого будет содержать текстовое
  fclose(f);                // описание ошибки, ее код и т.п.,
  system("pause");          // а на верхнем уровне его обрабатывать
  exit(7);
}



long * VarPointer(const char * name)
{
  if (!*name)
    Error("Syntax error.", 0);

  for (int i = 0; i < n; i++)
    if (strcmp(v[i].name, name) == 0)
      return &v[last = i].value;

  if (n >= MAX_VARS - 1)
    Error("Too many variables.", 0);

  strcpy(v[n].name, name);
  last = n;
  return &v[n++].value;
}

long VarValue(const char * name)
{
  if (!*name)
    Error("Syntax error.", 0);

  for (int i = 0; i < n; i++)
    if (strcmp(v[i].name, name) == 0)
      return v[i].value;

  Error("Undefined identifier \'%s\'.", name);
}

void VarPrint(void)
{
  if (last < 0)
    printf("No variables defined yet.\n");
  else
    printf("  %s = %ld\n", v[last].name, v[last].value);
}

void VarPrintAll(void)
{
  if (!n)
    printf("No variables defined yet.\n");
  else
    for (int i = 0; i < n; i++)
      printf("%s = %ld\n", v[i].name, v[i].value);
}



inline void Get(void)
{
  c = fgetc(f);
}



// Levels III-VI

long ProcC(void)
{
  long x = 0;
  while (c >= '0' && c <= '9')    // пока идут цифры
  {
    x *= 10;                      // собираем из них число
    x += c - '0';                 // по правилам позиционной
    Get();                        // системы счисления
  }
  return x;
}

long ProcI(void)
{
  char s[MAX_ID_LEN + 1] = {0};
  int i = 0;
  while ((c >= 'a' && c <= 'z') || c == '_' ||   // если это буква
    (i && c >= '0' && c <= '9'))                 // или не первая цифра
  {
    if (i >= MAX_ID_LEN)                         // и длина не превысила
      Error("Too long identifier \'%s\'.", s);   // допустимую, то
    s[i++] = c;                                  // собираем их
    Get();
  }
  if (!i)                                        // нашли что-нибудь?
    Error("Identifier missing.", 0);
  return VarValue(s);                 // возвращаем значение переменной
}

long * ProcL(void)
{
  char s[MAX_ID_LEN + 1] = {0};                  // аналогично
  int i = 0;
  while ((c >= 'a' && c <= 'z') || c == '_' ||
    (i && c >= '0' && c <= '9'))
  {
    if (i >= MAX_ID_LEN)
      Error("Too long identifier \'%s\'.", s);
    s[i++] = c;
    Get();
  }
  if (!i)
    Error("Identifier missing.", 0);
  return VarPointer(s);               // но возвращаем адрес переменной
}


// Level II

long ProcM(void)
{
  long x;

  if (c == '(')                    // выражение в скобках
  {
    Get();
    x = ProcE();                      // вычисляем
    if (c != ')')                     // скобку закрыть не забыли?
      Error("\')\' missing.", 0);
    Get();
  }
  else
    if (c == '-')                  // унарный минус
    {
      Get();
      x = -ProcM();                   // вычисляем
    }
    else
      if (c >= '0' && c <= '9')    // константа
        x = ProcC();                  // находим
      else
        if ((c >= 'a' && c <= 'z') || c == '_')  // переменная
          x = ProcI();                              // берем значение
        else
          Error("Syntax error.", 0);  // неизвестно что

  return x;            // возвращаем найденное значение
}

long ProcT(void)
{
  long x = ProcM();               // находим операнд
  while (c == '*' || c == '/')    // и пока идут операции
  {
    char p = c;                   // запоминаем операцию
    Get();
    if (p == '*')                 // и находим следующий операнд
      x *= ProcM();               // выполняя умножение на него
    else
      x /= ProcM();               // или деление
  }
  return x;
}

long ProcE(void)
{
  long x = ProcT();               // аналогично
  while (c == '+' || c == '-')
  {
    char p = c;
    Get();
    if (p == '+')
      x += ProcT();
    else
      x -= ProcT();
  }
  return x;
}


// Level I

void ProcS(void)
{
  long * p = ProcL();             // левый операнд - идентификатор
  if (c != '=')                   // потом =
    Error("\'=\' missing.", 0);
  Get();                          // затем находим значение
  *p = ProcE();                   // выражения - правый операнд
  if (c != ';')                   // дальше должен быть ;
    Error("\';\' missing.", 0);
  Get();
}


// High level

void Run(void)
{
  int i;
  Get();                          // берем первый символ и
  for (i = 0; ; i++)          // в цикле выполняем:
  {
    while (c >= 0 && c <= ' ')    // поглощаем пустые символы
      Get();                      // до, между и после операторов
    if (c == EOF)                 // и если не конец файла
      break;
    printf("Operator %2d: ", i+1);
    ProcS();                      // то разбираем следующий оператор
    VarPrint();                   // и выводим результат для контроля
  }
  printf("Result (%d variables defined in %d operators):\n", n, i);
  VarPrintAll();
}


int main(int argc, char ** argv)
{
  printf("\n");
  if (argc < 2)
  {
    printf("Command line parameter (file name) missing.\n");
    system("pause");
    return 1;
  }

  f = fopen(argv[1], "rb");
  if (!f)
  {
    printf("Can't open file \'%s\'.\n", argv[1]);
    system("pause");
    return 2;
  }

  Run();             // выполняем разбор программы

  system("pause");
  fclose(f);
  return 0;
}