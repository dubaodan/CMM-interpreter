#include <iostream>
#include <vector>
#include "headers/DataStruct.h"

//源代码文件
#define source "../source.txt"

//目标文件
#define compile "/Users/Lyra/lesson/compile.txt"

#define MaxLengthEachLine 200
#define MaxTokenLength 64

#define ReserveWordNum 15
#define OperatorNum 42

using namespace std;

enum EHMethods{
    ignoreLine,
    ignoreToken
};

//源文件行序列
struct line {
    char *content;
    int no;
    line *next;
};

//保留字表
static char reserveWord[ReserveWordNum][20] = {
    "break", "case", "char", "continue","default", "do", "else",
     "for",  "if", "int",  "real", "return", "switch", "void", "while"
};

//界符运算符表
static char operatorOrDelimiter[OperatorNum][5] = {
    "+", "-", "*", "/", "<", "<=", ">", ">=", "=", "==","<>", ";", "(", ")", ",", "\"", "\'", "\n",
    "\r", "\t", "++", "--","&", "&&", "|", "||", "<<", ">>","[", "]", "{", "}", "!", ":", "?", ".",
    "%", "+=", "-=", "*=","/=","%="
};

//标识符表

extern vector<Identifier> vIdentifier;
extern vector<Const> vConst;
extern vector<String> vString;
extern vector<Token> vToken;

//常数表

int searchReserve(char *s) {
    for (int i = 0; i < ReserveWordNum; i++)
        if (strcmp(reserveWord[i], s) == 0)
            return i + 1;//返回种别码
    return -1;
}

bool IsLetter(char letter) {
    return (letter >= 'a' && letter <= 'z') || (letter >= 'A' && letter <= 'Z') ;
}

bool IsUnderline(char letter) {
    return letter == '_';
}

bool IsDigit(char digit) {
    return digit >= '0' && digit <= '9';
}

bool IsDecimalPoint(char point){
    return point == '.';
}

bool IsSign(char letter){
    return letter == '-' || letter == '+';
}

bool IsExp(char letter){
    return letter == 'e' || letter == 'E';
}

bool IsHexadecimal(char first, char second){
    return first == '0' && second == 'x';
}

int Analyse(line *l, char *token, int &pointer, char *error, char &errorCh) {
    int i, count = 0, classNo = 0;
    char ch;
    ch = l->content[pointer];
    if (ch == '\0')
        return 0;

    while (ch == ' ' || ch == '\t') {
        pointer++;
        ch = l->content[pointer];
        if (ch == '\0')
            return 0;
    }
    for (i = 0; i<MaxTokenLength; i++)
        token[i] = '\0';

    if (IsLetter(ch)) { //为字母
         do{ //字母或数字
             if(count == MaxTokenLength-1){
                 cout<<"错误：标识符超出最大长度，行"<<l->no<<"，"<<pointer-1<<endl;
                 strcpy(error,"错误：标识符超出最大长度，行");
                 return -1;
             }
             token[count] = ch;
             count++;pointer++;
             ch = l->content[pointer];
         }while (IsLetter(ch) || IsDigit(ch) || IsUnderline(ch));
         if(IsUnderline(token[count-1])){    //以下划线结尾，出错
              cout<<"错误：标识符以下划线结尾，行"<<l->no<<"，"<<pointer-1<<endl;
              strcpy(error,"错误：标识符以下划线结尾，行");
              return -1;
         }
         token[count] = '\0';
         classNo = searchReserve(token);//查保留字表
         if (classNo == -1) //非保留字
            classNo = 100;  //标识符
         return classNo;
    } else if (IsDigit(ch) || IsDecimalPoint(ch)) {//为数字
        if (IsHexadecimal(ch,l->content[pointer+1])){
            token[count] = ch;
            count++;pointer++;
            token[count] = l->content[pointer];
            count++;pointer++;
            ch = l->content[pointer];
        }

        while (IsDigit(ch)) {//为数字
            token[count] = ch;
            count++;pointer++;
            ch = l->content[pointer];
        }

        if (IsDecimalPoint(ch)){
            token[count] = ch;
            count++;pointer++;
            ch = l->content[pointer];

            while (IsDigit(ch)){
                token[count] = ch;
                count++;pointer++;
                ch = l->content[pointer];
            }
            if(IsDecimalPoint(ch)){
                cout<<"错误：错误的数字，行"<<l->no<<"，"<<pointer-1<<endl;
                strcpy(error,"错误：错误的数字，行");
                return -1;
            }
        }
        if (IsExp(ch)){
            token[count] = ch;
            count++;pointer++;
            ch = l->content[pointer];
            if(IsSign(ch)){
                token[count] = ch;
                count++;pointer++;
                ch = l->content[pointer];
            }
            while (IsDigit(ch)){
                token[count] = ch;
                count++;pointer++;
                ch = l->content[pointer];
            }
            if(IsDecimalPoint(ch)){
                cout<<"错误：错误的数字，行"<<l->no<<"，"<<pointer-1<<endl;
                strcpy(error,"错误：错误的数字，行");
                return -1;
            }
        } else if (IsLetter(ch) || IsUnderline(ch)){
            cout<<"错误：错误的标识符或数字，行"<<l->no<<"，"<<pointer-1<<endl;
            strcpy(error,"错误：错误的标识符或数字，行");
            return -1;
        }
        token[count] = '\0';
        return 99;//常数
    } else if (  ch == '(' || ch == ')' || ch == ';' || ch == ':' || ch == ',' || ch == '.' || ch == '['
               || ch == ']' || ch == '{' || ch == '}' || ch == '!' || ch == '?' || ch == '\'') {//单一前缀符号
        token[0] = ch;
        token[1] = '\0';
        for (i = 0; i<OperatorNum; i++) {//查运算符界符表
            if (strcmp(token, operatorOrDelimiter[i]) == 0) {
                classNo = ReserveWordNum + 1 + i;
                break;
            }
        }
        pointer++;
        return classNo;
    }

    else if(ch == '\"'){
        token[count] = ch;
        count++;pointer++;
        while (l->content[pointer] != '\"'){
            token[count] = l->content[pointer];
            if(token[count] == '\0'){
                cout<<"错误：字符串缺少引号，行"<<l->no<<"，"<<pointer-1<<endl;
                strcpy(error,"错误：字符串缺少引号，行");
                return -1;
            }
            count++;pointer++;
        }
        token[count] = l->content[pointer];
        pointer++;
        return 98;
    }

    //单双目运算符混合
    else if (ch == '+'){//+,++,+=
        ch = l->content[++pointer];
        if (ch == '+')
            classNo = ReserveWordNum + 21;
        else if(ch == '='){
            classNo = ReserveWordNum + 38;
        } else {
            pointer--;
            classNo = ReserveWordNum + 1;
        }
        pointer++;
        return classNo;
    } else if (ch == '-'){//-,--,-=
        ch = l->content[++pointer];
        if (ch == '-')
            classNo = ReserveWordNum + 22;
        else if(ch == '='){
            classNo = ReserveWordNum + 39;
        } else {
            pointer--;
            classNo = ReserveWordNum + 2;
        }
        pointer++;
        return classNo;
    } else if (ch == '*'){//*,*=
        ch = l->content[++pointer];
        if (ch == '=')
            classNo = ReserveWordNum + 40;
        else {
            pointer--;
            classNo = ReserveWordNum + 3;
        }
        pointer++;
        return classNo;
    } else if (ch == '/'){// /,/=
        ch = l->content[++pointer];
        if (ch == '=')
            classNo = ReserveWordNum + 41;
        else {
            pointer--;
            classNo = ReserveWordNum + 4;
        }
        pointer++;
        return classNo;
    } else if (ch == '%'){//%,%=
        ch = l->content[++pointer];
        if (ch == '=')
            classNo = ReserveWordNum + 42;
        else {
            pointer--;
            classNo = ReserveWordNum + 37;
        }
        pointer++;
        return classNo;
    } else if (ch == '<') {//<,<=,<<,<>
        ch = l->content[++pointer];
        if (ch == '=')
            classNo = ReserveWordNum + 6;
        else if (ch == '<')
            classNo = ReserveWordNum + 27;
        else if (ch == '>')
            classNo = ReserveWordNum + 11;
        else {
            pointer--;
            classNo = ReserveWordNum + 5;
        }
        pointer++;
        return classNo;
    } else  if (ch == '>') {//>,>=,>>
        ch = l->content[++pointer];
        if (ch == '=')
            classNo = ReserveWordNum + 8;
        else if (ch == '>')
            classNo = ReserveWordNum + 28;
        else {
            pointer--;
            classNo = ReserveWordNum + 7;
        }
        pointer++;
        return classNo;
    } else  if (ch == '=') {//=.==
        ch = l->content[++pointer];
        if (ch == '=')
            classNo = ReserveWordNum + 10;
        else {
            pointer--;
            classNo = ReserveWordNum + 9;
        }
        pointer++;
        return classNo;
    } else  if (ch == '&') {//&,&&
        ch = l->content[++pointer];
        if (ch == '&')
            classNo = ReserveWordNum + 24;
        else {
            pointer--;
            classNo = ReserveWordNum + 23;
        }
        pointer++;
        return classNo;
    } else  if (ch == '|') {//|,||
        ch = l->content[++pointer];
        if (ch == '|')
            classNo = ReserveWordNum + 26;
        else {
            pointer--;
            classNo = ReserveWordNum + 25;
        }
        pointer++;
        return classNo;
    }

    //转义字符检查
    else if(ch == '\\'){
        ch = l->content[++pointer];
        if(ch == 'n')
            classNo = ReserveWordNum + 18;
        else if(ch == 'r')
            classNo = ReserveWordNum + 19;
        else if(ch == 't')
            classNo = ReserveWordNum + 20;
        else{
            cout<<"错误：不能识别的符号，行"<<l->no<<"，"<<pointer-1<<"："<<ch<<endl;
            strcpy(error,"错误：不能识别的符号，行");
            errorCh = '\\';
            return -2;
        }
        pointer++;
        return classNo;
    }
    else if (IsUnderline(ch)){
        cout<<"错误：错误的标识符，行"<<l->no<<"，"<<pointer-1<<endl;
        strcpy(error,"错误：错误的标识符，行");
        return -1;
    } else {//不能识别，出错
        cout<<"错误：不能识别的符号，行"<<l->no<<"，"<<pointer-1<<"："<<ch<<endl;
        strcpy(error,"错误：不能识别的符号，行");
        errorCh = ch;
        pointer++;
        return -2;
    }
    return -1;
}

int Lexical()
{
    char token[MaxTokenLength] = { 0 };
    int syn;
    int pProject = 0;//源程序指针
    int errors = 0;
    FILE *fp, *fp1;
    if ((fp = fopen(source, "r")) == nullptr) {//打开源程序
        cout << "can't open this file: "<<source;
        return 0;
    }

    line *lTable = static_cast<line*>(malloc(sizeof(line)));
    line *lRear=lTable, *lPointer;
    lTable->next= nullptr;
    lTable->no=0;

    char lineBuffer[MaxLengthEachLine];
    int pBuf=0;
    char charReadIn;
    while ((charReadIn = static_cast<char>(fgetc(fp)))) {

        if((charReadIn == '\n')|| charReadIn == EOF) { //完成一行的读取
            auto *newLine = static_cast<line*>(malloc(sizeof(line)));
            newLine->content = static_cast<char*>(malloc(sizeof(char)*(pBuf+1)));
            lineBuffer[pBuf]='\0';
            strcpy(newLine->content,lineBuffer);
            newLine->next= nullptr;
            newLine->no=lRear->no+1;
            lRear->next=newLine;
            lRear=newLine;
            if(charReadIn == EOF)
                break;
            for (int i = 0; i < pBuf ; ++i) //清空行缓存
                lineBuffer[i]='\0';
            pBuf=0;
            continue;
        }
        lineBuffer[pBuf] = charReadIn;
        pBuf++;
    }

    //去除注释
    line *lPrePointer=lTable;
    lPointer=lTable->next;
    bool inComment = false;
    bool needDelete;
    int commentLineNumber = 0;
    while (lPointer != nullptr) {
        needDelete = false;
        if (strcmp(lPointer->content, "\0") == 0) //空行
            needDelete = true;
        else if (strlen(lPointer->content) < 2 && !inComment) {  //一行内容短于注释符号
            needDelete = false;
        } else if (inComment){  //在块注释中间
            needDelete = true;
            int i = 0 , j = 1;
            while (lPointer->content[j]!='\0'){
                if (lPointer->content[i] == '*' &&lPointer->content[j] == '/'){ //判断是否是块注释的最后一行
                    inComment = false;
                    if(lPointer->content[j+1] == '\0')
                        break;
                    else {  //保留这一行中剩下的内容
                        char remain[MaxLengthEachLine] = "\0";
                        int m = 0;
                        j++;
                        while((remain[m] = lPointer->content[j]) != '\0'){
                            m++;
                            j++;
                        }
                        strcpy(lPointer->content,remain);
                        needDelete = false;
                    }
                    break;
                }
                i++;j++;
            }
        } else if (lPointer->content[0] == '/' && lPointer->content[1] == '/') { //行注释
            needDelete = true;
        } else { //块注释开始，或寻找行中间是否有行注释
            int i = 0, j = 1;
            while (lPointer->content[j]!='\0'){
                if (lPointer->content[i] == '/' && lPointer->content[j] == '*'){
                    commentLineNumber = lPointer->no;
                    needDelete = true;
                    inComment = true;

                    char remain[MaxLengthEachLine] = "\0";
                    int m = 0, n=0;
                    if(i != 0){
                        needDelete = false;
                        while(n != i){
                            remain[m] = lPointer->content[n];
                            m++;n++;
                        }
                    }

                    i+=2;j+=2;
                    while (lPointer->content[j]!='\0'){
                        if (lPointer->content[i] == '*' &&lPointer->content[j] == '/'){ //判断是否是块注释的最后一行
                            inComment = false;
                            if(lPointer->content[j+1] == '\0')
                                break;
                            else {  //保留这一行中剩下的内容
                                j++;
                                while((remain[m] = lPointer->content[j]) != '\0'){
                                    m++;
                                    j++;
                                }
                                strcpy(lPointer->content,remain);
                                needDelete = false;
                            }
                            break;
                        }
                        i++;j++;
                    }
                    strcpy(lPointer->content,remain);
                    break;
                } else if (lPointer->content[i] == '/' && lPointer->content[j] == '/'){
                    needDelete = true;
                    char remain[MaxLengthEachLine] = "\0";
                    int m = 0, n=0;
                    if(i != 0){
                        needDelete = false;
                        while(n != i){
                            remain[m] = lPointer->content[n];
                            m++;n++;
                        }
                    }
                    strcpy(lPointer->content,remain);
                    break;
                }
                i++;j++;
            }
        }

        if(needDelete){
            lPrePointer->next = lPointer->next;
            free(lPointer);
            lPointer = lPrePointer->next;
        } else {
            lPrePointer=lPointer;
            lPointer=lPointer->next;
        }
    }
    if (inComment) {
        cout<<"块注释错误，找不到\"*/\"，用于匹配第"<<commentLineNumber<<"行的/*";
        errors++;
        return errors;
    }

    fclose(fp);

    //打开要写入的文件
//    if ((fp1 = fopen(compile, "w+")) == nullptr) {
//        cout << "can't open this file";
//        exit(0);
//    }

    pProject = 0;
    char error[100], errorCh;
    lPointer = lTable->next;

    EHMethods method  = ignoreToken;
    while (lPointer != nullptr) {
        syn = Analyse(lPointer, token, pProject, error, errorCh);
        switch (syn){
            case 0:
                lPointer = lPointer->next;
                pProject = 0;
                break;
            case 100:
            {
                string s(token);
                bool find= false;
                int index;
                for (auto & iter : vIdentifier){
                    if (iter.name == s){
                        find = true;
                        index = iter.index;
                        break;
                    }
                }
                if (!find) {
                    index = vIdentifier.back().index+1;
                    vIdentifier.emplace_back(s,index);
                }
                vToken.emplace_back(lPointer->no,100,index);
                break;
            }
            case 99:
            {
                string s(token);
                bool find= false;
                int index;
                for (auto & iter : vConst){
                    if (iter.value == s){
                        find = true;
                        index = iter.index;
                        break;
                    }
                }

                if (!find) {
                    index = vConst.back().index+1;
                    vConst.emplace_back(s,index);
                }
                vToken.emplace_back(lPointer->no,99,index);
                break;
            }
            case 98:
            {
                string s(token);
                int index;
                index = vString.back().index+1;
                vString.emplace_back(s,index);
                vToken.emplace_back(lPointer->no,98,index);
                break;
            }

            default:
                break;
        }

        if (syn >= 1 && syn <= 65) {//保留字
            Token t = Token(lPointer->no,syn,0);
            vToken.push_back(t);
        } else if (syn < 0){
            errors++;
            //写入文件
//            switch (syn){
//                case -1:
//                    fprintf(fp1,"%s%d，%d\n", error,lPointer->no,pProject);
//                    break;
//                case -2:
//                    fprintf(fp1,"%s%d，%d：%c\n", error,lPointer->no,pProject,errorCh);
//                    break;
//                default:
//                    break;
//            }
            switch (method){
                case ignoreLine:
                    lPointer = lPointer->next;
                    pProject = 0;
                    break;
                case ignoreToken:
                    while (lPointer->content[pProject] != ' ' ){
                        if(lPointer->content[pProject] == '\0'){
                            pProject = 0;
                            lPointer = lPointer->next;
                            break;
                        }
                        pProject++;
                    }
                    break;
            }
        } else if(syn < 0 || syn > 100){
            cout<< "lexical analysis error: unknown class no."<<endl;
            exit(1);
        }
    }

//    fclose(fp1);
    vToken.emplace_back(Token(vToken.back().line,101,0));
    return errors;
}