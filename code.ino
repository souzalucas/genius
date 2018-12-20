//#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <DS1307.h>
#include <EEPROM.h>
#include <string.h>

#define cpl_bit(Y,bit_x)(Y^=(1<<bit_x)) //troca o estado do bit x da variável Y
#define tst_bit(Y,bit_x)(Y&(1<<bit_x)) //testa o bit x da variável Y (retorna 0 ou 1)
#define set_bit(y,bit)  (y|=(1<<bit)) //coloca em 1 o bit x da variável Y
#define clr_bit(y,bit)  (y&=~(1<<bit))  //coloca em 0 o bit x da variável Y

//-------------------------------------Inicialização-dos-LEDs--------------------------------------------------
#define LED3 PD4
#define LED2 PD5
#define LED1 PD6
#define LED0 PD7
//-----------------------------------Final-da-Inicialização-dos-LEDs--------------------------------------------

//-------------------------------------Inicialização-dos-botoes-------------------------------------------------
#define BOTAO0 PD4
#define BOTAO1 PD5
#define BOTAO2 PD6
#define BOTAO3 PD7
//-----------------------------------Final-da-Inicialização-dos-botoes------------------------------------------

//-------------------------------------Inicialização-dos-SWITCHs-para-troca-de-estado-do-display-----------------
#define SWITCH1 PD2
#define SWITCH2 PD3
//-----------------------------------Final-da-Inicialização-dos-SWITCHs-para-troca-de-estado-do-display----------

//------------------------------------Inicialização-dos-segmentos-do-display-------------------------------------
#define CD PB0
#define CE PB1
#define C  PB2
#define BE PB3
#define B  PB4
#define BD PB5
#define M  PC3
//------------------------------------Inicialização-dos-segmentos-do-display-------------------------------------

//------------------------------------Inicialização-das-variaveis-globais----------------------------------------
int sequencia[50] = {};
int pos = 0;
int verdade = 1;
int quantidade = 0;
int vetorSOM[2] = {294,392};
int vetorLED[4] = {7,6,5,4};
int vetorBOTAO[4] = {7,6,5,4};
int acao = 0;
int flag = 4;
int teste = 0, num1 = 0, num2 = 0;
char buz = 16;

char bufferName[15];
char bufferNivel[15];
char nickName[70];
char *data;
long int endEeprom = 2;
int stringTam = 0;
int pontuacao = 0;
int stoper = 0;
int stoper2 = 0;
//-----------------------------------Finalização-da-Inicialização-das-variaveis-globais----------------------------

//Função para randomizar os Leds acesos
int rando() {
  if(strcmp(bufferNivel,"1") == 0){
    int numRan = random(4, 8);
    sequencia[pos] = numRan;
    pos++;
  }else if(strcmp(bufferNivel,"2") == 0){
    int numRan = random(4, 8);
    sequencia[pos] = numRan;
    pos++;
    numRan = random(4, 8);
    sequencia[pos] = numRan;
    pos++;
  }else if(strcmp(bufferNivel,"3") == 0){
    int numRan = random(4, 8);
    sequencia[pos] = numRan;
    pos++;
    numRan = random(4, 8);
    sequencia[pos] = numRan;
    pos++;
    numRan = random(4, 8);
    sequencia[pos] = numRan;
    pos++;
  }
  int tamanho = pos;
  PCICR = 0<<PCIE2;
  sei();
  for(int i = 0; i < tamanho; i++) {
    cpl_bit(PORTD,sequencia[i]);
    _delay_ms(200);
    cpl_bit(PORTD,sequencia[i]);
    _delay_ms(200);
  }
  PCICR = 1<<PCIE2;
  sei();
}
//Interrupção para os botoes acenderem os LEDs
ISR(PCINT2_vect)//quando houver mais de um pino que possa gerar a interrup��o � necess�rio testar qual foi
{
     //botao azul
    while(!tst_bit(PIND,vetorBOTAO[0])){
      flag = 0;
    }
    while(!tst_bit(PIND,vetorBOTAO[1])){
      flag = 1;
    }
    while(!tst_bit(PIND,vetorBOTAO[2])){
      flag = 2;
    }
    while(!tst_bit(PIND,vetorBOTAO[3])){
      flag = 3;
    } 
}

//------------------------------------Função-de-cria-as-musicas---------------------------------------

char* musica[] = {"La","Re","Fa","Sol","La","Re", "Fa", "Sol", "Mi", "Pausa", "Sol", "Do", "Fa", "Mi", "Sol", "Do", "Fa", "Mi", "Re", "Fim"}; //Game of Thrones
int duracao[] = {700, 500, 300, 250, 250, 300, 200, 200, 700, 200, 500, 500, 200, 200, 200, 500, 200, 200, 500};
//Game of Thrones
char* starwars[] = {"La","Pausa","La","Pausa","La","Pausa","Fa","Do","La","Pausa","Fa","Do","La","Pausa","Mi","Pausa","Mi","Pausa","Mi","Pausa","Fa","Do","Sol","Pausa","Fa","Do","La","Pausa","La","Pausa","La","Pausa","La","Pausa","La","Pausa","Sol#","Pausa","Sol","Fa#","Fa","Fa#","Fim"}; //Marcha Imperial
int dur[] = {400, 100, 400, 100, 400, 100, 300, 200, 300, 100, 300, 200, 300, 200, 400, 100, 400, 100, 400, 100, 300, 300, 200, 100 , 300, 300, 200, 200, 400, 50, 400, 50, 400, 50, 400, 50, 300, 50, 300, 200, 200, 200};
//Marcha Imperial

void tocar(char* mus[], int tempo[]){
  int tom = 0;
  for(int i = 0; mus[i]!="Fim";i++){
    if(mus[i] == "Do") tom = 262;
    if(mus[i] == "Re") tom = 294;
        if(mus[i] == "Mi") tom = 330;
        if(mus[i] == "Fa") tom = 349;
        if(mus[i] == "Sol") tom = 392;
        if(mus[i] == "La") tom = 440;
        if(mus[i] == "Si") tom = 494;
        if(mus[i] == "Do#") tom = 528;
        if(mus[i] == "Re#") tom = 622;
        if(mus[i] == "Fa#") tom = 370;
        if(mus[i] == "Sol#") tom = 415;
        if(mus[i] == "La#") tom = 466;
        if(mus[i] == "Pausa") tom = 0;
    tone(buz, tom, tempo[i]);
    delay(tempo[i]);
  }
}
//----------------------------------Fim-da-Função-de-cria-as-musicas---------------------------------------

//----------------------------------Fim-da-Função-que-cria-um-mapa-de-numeros-para-o-display---------------
void acende(int numero){
  if(numero == 0){
    clr_bit(PORTB, CD);
    clr_bit(PORTB, CE);
    clr_bit(PORTB, BD);
    clr_bit(PORTB, BE);
    clr_bit(PORTB, C);
    clr_bit(PORTB, B);
  }else if(numero == 1){
    clr_bit(PORTB, CE);
    clr_bit(PORTB, BE);
  }else if(numero == 2){
    clr_bit(PORTB, CD);
    clr_bit(PORTB, BE);
    clr_bit(PORTB, C);
    clr_bit(PORTB, B);
    clr_bit(PORTC, M);
  }else if(numero == 3){
    clr_bit(PORTB, C);
    clr_bit(PORTB, CE);
    clr_bit(PORTB, BE);
    clr_bit(PORTC, M);
    clr_bit(PORTB, B);
  }else if(numero == 4){
    clr_bit(PORTB, BD);
    clr_bit(PORTB, CE);
    clr_bit(PORTB, BE);
    clr_bit(PORTC, M);
  }else if(numero == 5){
    clr_bit(PORTB, C);
    clr_bit(PORTB, CE);
    clr_bit(PORTC, M);
    clr_bit(PORTB, BD);
    clr_bit(PORTB, B);
  }else if(numero == 6){
    clr_bit(PORTB, C);
    clr_bit(PORTB, BD);
    clr_bit(PORTC, M);
    clr_bit(PORTB, CE);
    clr_bit(PORTB, CD);
    clr_bit(PORTB, B);
  }else if(numero == 7){
    clr_bit(PORTB, B);
    clr_bit(PORTB, CE);
    clr_bit(PORTB, BE);
  }else if(numero == 8){
    clr_bit(PORTB, C);
    clr_bit(PORTB, CE);
    clr_bit(PORTB, CD);
    clr_bit(PORTC, M);
    clr_bit(PORTB, BE);
    clr_bit(PORTB, BD);
    clr_bit(PORTB, B);
  }else if(numero == 9){
    clr_bit(PORTB, B);
    clr_bit(PORTB, BE);
    clr_bit(PORTB, BD);
    clr_bit(PORTC, M);
    clr_bit(PORTB, CE);
  }    
}
//----------------------------------Fim-da-Função-que-cria-um-mapa-de-numeros-para-o-display---------------

//---------------------Interrupção-por-tempo-para-chaveamento-de-contexto-do-display-----------------------
ISR(TIMER1_COMPA_vect) {
  set_bit(PORTB, CD);
  set_bit(PORTB, CE);
  set_bit(PORTB, BD);
  set_bit(PORTB, BE);
  set_bit(PORTB, C);
  set_bit(PORTB, B);
  set_bit(PORTC, M);
  set_bit(PORTD, SWITCH2);
  set_bit(PORTD, SWITCH1);
  if(teste == 0){
    clr_bit(PORTD, SWITCH2);
    acende(num1);
  }else if(teste == 1){
   clr_bit(PORTD, SWITCH1);
   acende(num2);
  }else if(teste >= 2){
    teste = -1;
  }
  teste++;
}
//---------------------Interrupção-por-tempo-para-chaveamento-de-contexto-do-display-----------------------

//--------------------inicio-Funções-de-leitura-e-escrita-da-EEPROM----------------------------------------
void escreveInt(int endereco1, int endereco2, int valor){ // Escreve um inteiro de 2 bytes na EEPROM
  int valorAtual = lerInt(endereco1,endereco2); // Lemos o valor inteiro da memória
  if (valorAtual == valor){ // Se o valor lido for igual ao que queremos escrever não é necessário escrever novamente
    return;
  }
  else{ // Caso contrário "quebramos nosso inteiro em 2 bytes e escrevemos cada byte em uma posição da memória
      byte primeiroByte = valor&0xff; //Executamos a operação AND de 255 com todo o valor, o que mantém apenas o primeiro byte
      byte segundoByte = (valor >> 8) &0xff; // Realizamos um deslocamento de 8 bits para a direita e novamente executamos um AND com o valor 255, o que retorna apenas o byte desejado
      EEPROM.write(endereco1,primeiroByte); // Copiamos o primeiro byte para o endereço 1
      EEPROM.write(endereco2,segundoByte); // Copiamos o segundo byte para o endereço 2
  }
}

int lerInt(int endereco1, int endereco2){ // Le o int armazenado em dois endereços de memória
  int valor = 0; // Inicializamos nosso retorno
  byte primeiroByte = EEPROM.read(endereco1); // Leitura do primeiro byte armazenado no endereço 1
  byte segundoByte = EEPROM.read(endereco2); // Leitura do segundo byte armazenado no endereço 2
  valor = (segundoByte << 8) + primeiroByte; // Deslocamos o segundo byte 8 vezes para a esquerda ( formando o byte mais significativo ) e realizamos a soma com o primeiro byte ( menos significativo )
  return valor; // Retornamos o valor da leitura
 
}

void escreveString(int enderecoBase, String mensagem){ // Salva a string nos endereços de forma sequencial
  if (mensagem.length()>EEPROM.length() || (enderecoBase+mensagem.length()) >EEPROM.length() ){ // verificamos se a string cabe na memória a partir do endereço desejado
    Serial.println(enderecoBase+mensagem.length());
    Serial.println(EEPROM.length());
    Serial.println ("A sua String não cabe na EEPROM"); // Caso não caiba mensagem de erro é mostrada
  }
  else{ // Caso seja possível armazenar 
    for (int i = 0; i<mensagem.length(); i++){ 
       EEPROM.write(enderecoBase,mensagem[i]); // Escrevemos cada byte da string de forma sequencial na memória
       enderecoBase++; // Deslocamos endereço base em uma posição a cada byte salvo
    }
    EEPROM.write(enderecoBase,'\0'); // Salvamos marcador de fim da string 
  }
}
 
String leString(int enderecoBase){
  String mensagem="";
  if (enderecoBase>EEPROM.length()){ // Se o endereço base for maior que o espaço de endereçamento da EEPROM retornamos uma string vazia
    return mensagem;
  }
  else { // Caso contrário, lemos byte a byte de cada endereço e montamos uma nova String
    char pos;
    do{
      pos = EEPROM.read(enderecoBase); // Leitura do byte com base na posição atual
      enderecoBase++; // A cada leitura incrementamos a posição a ser lida
      mensagem = mensagem + pos; // Montamos string de saídaa
    }
    while (pos != '\0'); // Fazemos isso até encontrar o marcador de fim de string
  }
  return mensagem; // Retorno da mensagem
}
//--------------------Fim-Funções-de-leitura-e-escrita-da-EEPROM-------------------------------------------

//---------------------Inicio-da-função-para-leitura-do-nome-do-usuario------------------------------------
void readName(){
 //escreveInt(0,1,2);
 endEeprom = lerInt(0,1);
Serial.println("");
Serial.println("Digite seu nome de usuario: ");
int i = 0;
  while(!stoper) {
    if (Serial.available() > 0) {
      Serial.readBytesUntil('\n', bufferName, sizeof(bufferName) / sizeof(char) );
      for(i = 0; bufferName[i] != '\0'; i++){  
        nickName[i] = bufferName[i];
        bufferName[i] = '\0';
      }
      stringTam = i;

      Serial.print("Pode começar o jogo: ");
      Serial.println(nickName);
      stoper = 1;
    }
  }
  stoper = 0;
  if(endEeprom >= 1024){
    endEeprom = 2;  
  }
  Serial.println("Escolha um dos niveis abaixo:");
  Serial.println("1: nivel 1");
  Serial.println("2: nivel 2");
  Serial.println("3: nivel 3");
  Serial.println("Digite aqui: ");

  while(!stoper2) {
    if (Serial.available() > 0) {
      Serial.readBytesUntil('\n', bufferNivel, sizeof(bufferNivel) / sizeof(char) );
      stoper2 = 1;
    }
  }
  if(strcmp(bufferNivel,"1") == 0){
    Serial.println("Você selecionou o nivel 1");
  }else if(strcmp(bufferNivel,"2") == 0){
    Serial.println("Você selecionou o nivel 2");
  }else if(strcmp(bufferNivel,"3") == 0){
    Serial.println("Você selecionou o nivel 3");
  }else {
    Serial.println("nivel invalido, Por padrão nivel gravado como 1");
    strcpy(bufferNivel,"1");
  }
  stoper2 = 0;
  
  randomSeed(millis());
  _delay_ms(500);
}
//---------------------Fim-da-função-para-leitura-do-nome-do-usuario---------------------------------------


DS1307 rtc(A4,A5); //inicialização das portas A4 e A5 para o I2C

void setup () {
    DDRD = 0b11111100;
    DDRB = 0b11111111;
    DDRC = 0b11111111;

    PORTD = 0b11110000;
    PORTC = 0b11111111;
    PORTB = 0b00111111;
    
    Serial.begin(9600);
    rtc.begin();
    rtc.halt(false);

    rtc.setDOW(THURSDAY);
    rtc.setTime(10, 52, 0);
    rtc.setDate(14, 12, 2018);

    rtc.setSQWRate(SQW_RATE_1);
    rtc.enableSQW(true);
      
}
char bigstring[40];  // enough room for all strings together

char* join3Strings(char* string1, char* string2, char* string3) {
 bigstring[0] = 0;          // start with a null string:
 strcat(bigstring, string1);   // add first string
 strcat(bigstring," | Pontuação:");
 strcat (bigstring, string2);
 strcat(bigstring," | Data:");
 strcat (bigstring, string3);
 strcat(bigstring," | Nivel:");
 strcat(bigstring, bufferNivel);

 return bigstring;
}
void loop() {
  readName();
  //tocar(musica, duracao);

  // TIMER 1
  OCR1A = 156; // comparador
  TCCR1A = (1<<WGM12); 
  TCCR1B = (1<<CS10) | (1<<CS12) & (0<<CS11); //TC1 com prescaler de 1024, a 16 MHz gera uma interrupção a cada 10 ms
  TIMSK1 = (1<<OCIE1A); // Bit 1 – OCIE1A
      
  PCICR = 1<<PCIE2;//habilita interrup��o por qualquer mudan�a de sinal no PORTD
  PCMSK2 = (1<<PCINT20)|(1<<PCINT21)|(1<<PCINT22)|(1<<PCINT23);//habilita os pinos PCINT8:11 para gerar interrup��o
    
  sei();            //habilita as interrup��es

  set_bit(PORTD, SWITCH1);
  set_bit(PORTD, SWITCH2);
  
  while(1){
      if(acao == 0){
        //etapa 1 : Gerar sequencia aleatoria
        _delay_ms(500);
        rando();
         acao= 1; 
      }
      while(acao == 1){
        //etapa 2

        while(quantidade < pos) {
          
          if(flag != 4) {
            _delay_ms(100);
            if(sequencia[quantidade] == vetorLED[flag]) {
              acao = 0;
            } else {
              acao = -1;  
            }
            quantidade++;
            flag = 4;
          }
          if (acao == -1) {
            break;
          }
        }
      }
      if(acao == 0){
        pontuacao++;
        num1++;
        if(num1 > 9){
          num1 = 0;
          num2++;
        }  
      }
      quantidade = 0;
      if(acao == -1){
        //etapa 3 : Game over
        
        Serial.println("");
        Serial.println("*************** GAME OVER ***************");
        Serial.println("");
        num1 = 0;
        num2 = 0;
        data = rtc.getDateStr(FORMAT_SHORT);
        char point[15];
        sprintf(point,"%d",pontuacao);
        char *envio = join3Strings(nickName, point, data);
        escreveString(endEeprom,envio);
        endEeprom+=70;
        escreveInt(0,1,endEeprom);
        PCICR = 0<<PCIE2;
        sei();
        for(int i = 0; i <= 5; i++){
          cpl_bit(PORTD,vetorLED[0]);
          cpl_bit(PORTD,vetorLED[1]);
          cpl_bit(PORTD,vetorLED[2]);
          cpl_bit(PORTD,vetorLED[3]);
          _delay_ms(200);
        }
          PCICR = 1<<PCIE2;
          sei();
        Serial.println("----------------PLACAR GERAL-------------------");
        int x = 2;
        do{
          Serial.println("Player:"+leString(x));
          _delay_ms(100);
          x+=70;
        }while(x < endEeprom);
        Serial.println("----------------PLACAR GERAL-------------------");
        exit(0);
      }
  }
}   
