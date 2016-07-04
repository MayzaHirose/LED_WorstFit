//Bibliotecas
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//Interfaces das Fun��es
int obterCampos(FILE* arqCat, char* strCampo, int* reg);
short obterRegistro(FILE* arqReg, char* strBuffer);
void receberDados(char* strBuffer);


//Cria��o de um tipo booleano para facilitar a manuten��o dos ID procurados na hora da remo��o
enum boolean {
    true = 1, false = 0
};
typedef enum boolean bool;


//Programa principal
int main(){

	//Vari�veis comuns
	int opcao = 0; //Opc�o do menu
	int campo = 0; //Contador de campos
	int contadorReg = 0; //Contador de registros
	int field_length = 0; //Armazena o tamanho do campo
	short tamanhoRegistro = 0; //Armazena o tamanho do registro
	short rec_length; //Armazena o tamanho do registro
	char filename[20], strCampo[256], strBuffer[512]; // Strings
	char* token; //Armazena parte de string ao utilizar strtok

	//Vari�veis para manuten��o da LED
	short LED = -1; //LED com valor inicial
	short byteOffset = 0; //Armazena o byte de in�cio do registro removido
	short tamanhoLED; //Armazena o tamanho do registro apontado pela LED
	short tamanhoByte; //Armazena o tamanho do registro apontado pelo byteoffset
	short valorApontado; //Valor do byteoffset visitado
	short sobra; //Armazena a sobra do registro quando inserimos um registro menor no espa�o
	short LEDatual;//Armazena o valor da LED que esta sendo verificada 
	short aux;
	

	//Vari�veis da Busca Sequencial para procurar o registro a ser removido
	char searchKey[10]; //ID a ser removido
	char* id; //ID do registro a ser comparado com o procurado
	bool matched = false; //Armazena o estado da procura

	//Ponteiros para os arquivos
	FILE* arqCat;
	FILE* arqReg;

	do{

		printf("\n\n           ---------------------------------------------------\n");
		printf("           |PROGRAMA PARA GERENCIAMENTO DE CATALOGO DE ALUNOS|\n");
		printf("           ---------------------------------------------------\n");
	    printf("Suas opcoes sao:\n\n");
	    printf("    1. Importar catalogo de alunos\n");
	    printf("    2. Inserir novo registro\n");
	    printf("    3. Remover registro existente\n");
	    printf("    4. Terminar o programa\n");
	    printf("\nDigite o numero da sua escolha: ");
	    scanf("%d", &opcao);
	    fflush(stdin);

	    switch(opcao){

			/* IMPORTA��O DE REGISTROS */
	        case 1:
	        	strCampo[0] = '\0';
	        	strBuffer[0] = '\0';


	       		printf("\nDigite o nome do catalogo: ");
   				gets(filename);
    			if((arqCat = fopen(filename, "r")) == NULL){
        			printf("*ERRO!");
       				break;
    			}else{
    				printf("*Arquivo Encontrado!*");}


	       		printf("\n\nDigite um nome para o novo Arquivo de Registros: ");
				gets(filename);
				if((arqReg = fopen(filename, "w+")) == NULL){
	    			printf("*ERRO!");
	    			break;
				}else{
					printf("*Arquivo Criado!*");}

				LED = -1;
				rewind(arqReg);
    			fwrite(&LED, sizeof(LED), 1, arqReg);

    			/*Faz a importa��o dos dados do catalogo para o arquivo de registro com os devidos ajustes*/
				field_length = obterCampos(arqCat, strCampo, &contadorReg);
				strcat(strBuffer,strCampo);
    			strcat(strBuffer,"|");

    			while(field_length > 0){
			        campo ++;
			        printf("\nCampo #%i = %s", campo, strCampo);
			        strCampo[0] = '\0';

			        if(!(campo % 5) ){
				        tamanhoRegistro = strlen(strBuffer);
				        fwrite(&tamanhoRegistro, sizeof(tamanhoRegistro), 1, arqReg);
				        fwrite(strBuffer, tamanhoRegistro, 1, arqReg);
				        strBuffer[0] = '\0';
			        }

			        field_length = obterCampos(arqCat, strCampo, &contadorReg);
			        strcat(strBuffer,strCampo);
			        strcat(strBuffer,"|");
				}
				
				printf("\n\nIMPORTACAO REALIZADA COM SUCESSO!");
	        break;
	        /* FIM IMPORTA��O */

	        /* INSER��O DE REGISTROS */
	        case 2:
	        	strCampo[0] = '\0';
	        	strBuffer[0] = '\0';

	        	//Recebe o novo registro
	        	receberDados(strBuffer);
	        	printf("\n\nDados recebidos: %s", strBuffer);

	        	//Verifica o valor da LED
	        	rewind(arqReg);
	        	fread(&LED, sizeof(LED), 1, arqReg);
   				printf("\n\nLED atual: %d", LED);

   				tamanhoRegistro = strlen(strBuffer);

   				//Se a LED n�o tem espa�o dispon�vel, p�e o registro no final no arquivo
   				if(LED == -1){
					printf("\n\nA LED nao tem espacos disponiveis!");
					printf("\nO novo registro sera gravado no final do arquivo.");

					//Posiciono o ponteiro de escrita no final do arquivo
		        	fseek(arqReg, 0, SEEK_END);

		        	//Escrevo o tamanho do registro e o registro no final do arquivo
		        	fwrite(&tamanhoRegistro, sizeof(tamanhoRegistro), 1, arqReg);
	        		fwrite(strBuffer, tamanhoRegistro, 1, arqReg);
				    }


        		else {
        			printf("\nA LED possui espaco disponivel!");

        			//Posiciono o ponteiro de leitura no byte apontado pela led. Tiro 2 pois quero ver os dois bytes antes do registro
					fseek(arqReg, LED-2, SEEK_SET);

					//Pego o tamanho do registro do espaco disponivel
					fread(&tamanhoLED, sizeof(tamanhoLED), 1, arqReg);

					printf("\n\nTamanho do registro que quero inserir: %d", tamanhoRegistro);
					printf("\nTamanho do registro disponivel na LED: %d", tamanhoLED);

        			//Verifico se o novo registro cabe no espa�o dispon�vel
        			if(tamanhoRegistro <= tamanhoLED){
        				printf("\n\nO novo registro cabe no espaco disponivel!");

						/*Calculo a sobra de espa�o. Subtraio -2 pois al�m do tamanho do registro novo,
						tem os dois bytes que armazenar�o o tamanho do registro novo*/
        				sobra = (tamanhoLED - tamanhoRegistro) - 2;
        				
						if(tamanhoRegistro == tamanhoLED || sobra <= 0){
							fseek(arqReg, LED+1, SEEK_SET);
        					fread(&valorApontado, sizeof(valorApontado), 1, arqReg);
        					fseek(arqReg, LED-2, SEEK_SET);
	        				fwrite(&tamanhoRegistro, sizeof(tamanhoRegistro), 1, arqReg);
			        		fwrite(strBuffer, tamanhoRegistro, 1, arqReg);
        					printf("\nNao sobrou espaco para reutilizar ja que o registro somado aos bytes de tamanho ocupara td registro");
        					//Fa�o a led apontar para o proximo registro disponivel, ja que este nao tem mais espa�o e nao precisa encaixar
        					rewind(arqReg);
        					fwrite(&valorApontado, sizeof(valorApontado), 1, arqReg);
        				}
        		
        				else{
        					printf("\nSobra: %d", sobra);
        					
	        				//Gravo o tamanho da sobra no registro do valor apontado pela LED
	        				fseek(arqReg, LED-2, SEEK_SET);
	        				fwrite(&sobra, sizeof(sobra), 1, arqReg);
	
	        				//Agora tenho que escrever o novo registro no final do espaco disponivel e esse espa�o vira um novo registro
	        				fseek(arqReg, LED+sobra, SEEK_SET);
	        				fwrite(&tamanhoRegistro, sizeof(tamanhoRegistro), 1, arqReg);
			        		fwrite(strBuffer, tamanhoRegistro, 1, arqReg);
	
			        		//Pego o valor apontado pelo byteoffset apontado pela LED
			        		fseek(arqReg, LED+1, SEEK_SET);
		        			fread(&valorApontado, sizeof(valorApontado), 1, arqReg);
	   						printf("\n\nByteoffset apontado pelo Byteoffset apontado pela LED: %d", valorApontado);
	
	   						/*Fa�o a led apontar para esse valor apontado pois abaixo vou fngir
						 	que a sobra � um novo espa�o removido que precisa ser encaixado na LED*/
	   						rewind(arqReg);
	   						fwrite(&valorApontado, sizeof(valorApontado), 1, arqReg);
	
	
			        		//O byteoffset agora � o meu "novo registro " mas na vdd � o registro que sobrou
			        		byteOffset = LED;
			        		//A LED � o valor que estava sendo apontado pela LED disponivel
			        		LED = valorApontado;
			        		
			        		printf("\n\nMinha 'nova' LED: %d", LED);
	
			        		/*Se o valor apontado pela LED era -1, o espacinho que sobrou entra na LED como se fosse o primeiro elemento da LED*/
			        		if(LED == -1){
			   					fseek(arqReg, byteOffset, SEEK_SET);
			   					fwrite("*",sizeof(char), 1, arqReg);
			   					fwrite(&LED, sizeof(LED), 1, arqReg);
	
			   					//LED recebe seu novo valor
			   				 	LED = byteOffset;
			   				 	rewind(arqReg);
			   				 	fwrite(&LED, sizeof(LED), 1, arqReg);
	
							/* Se n�o, verifica onde encaixar o pedacinho de registro que sobrou */
							}else{
	
				        		//Movo o ponteiro de leitura na posicao da LED atual. Tira 2 pois quero ver o tamanho do registro da LED
								fseek(arqReg, LED-2, SEEK_SET);
	
								//Pego o tamanho do espa�o do registro da LED
								fread(&tamanhoLED, sizeof(tamanhoLED), 1, arqReg);
								
								printf("\nTamanho do registro apontado pela 'nova LED'': %d", tamanhoLED);
	
								//Movo o ponteiro de leitura na posicao do valor "removido" mas tiro 2 para ver o tamanho disponivel (no caso o resto)
								fseek(arqReg, byteOffset-2, SEEK_SET);
	
								//Pego o tamanho do espa�o "removido"
								fread(&tamanhoByte, sizeof(tamanhoByte), 1, arqReg);
	
								printf("\n\nByteoffset do 'novo espaco disponivel': %d", byteOffset);
								printf("\nTamanho do espaco 'removido' que na vdd e a sobra: %d", tamanhoByte);
	
								//Se o tamanho da LED for maior que o tamanho do registro 'removido'
								if(tamanhoLED > tamanhoByte){
									//Verifico se o byteoffset da led aponta pra -1. +1 pois quero pular o *
									fseek(arqReg, LED+1, SEEK_SET);
									fread(&valorApontado, sizeof(valorApontado), 1, arqReg);
									fseek(arqReg, valorApontado-2, SEEK_SET);
									fread(&tamanhoLED, sizeof(tamanhoLED), 1, arqReg);
									LEDatual = LED;
									//Enquanto nao encontra um -1 ou o tamanho da led � maior que o tamanho do novo espaco
									while(valorApontado != -1 && tamanhoLED > tamanhoByte){
										//pego o valor apontado e vou pra ele e verifico de novo
										LEDatual = valorApontado;
										fseek(arqReg, LEDatual+1, SEEK_SET);
										fread(&valorApontado, sizeof(valorApontado), 1, arqReg);
																	
										fseek(arqReg, valorApontado-2, SEEK_SET);
										fread(&tamanhoLED, sizeof(valorApontado), 1, arqReg);
									}
									if(valorApontado == -1){
										fseek(arqReg, LEDatual+1, SEEK_SET);
										fwrite(&byteOffset, sizeof(byteOffset), 1, arqReg);
										fseek(arqReg, byteOffset, SEEK_SET);
										fwrite("*",sizeof(char), 1, arqReg);
										fwrite(&valorApontado, sizeof(valorApontado), 1, arqReg);
									}else{
										fseek(arqReg, LEDatual, SEEK_SET);
										fwrite("*",sizeof(char), 1, arqReg);
										fwrite(&byteOffset, sizeof(byteOffset), 1, arqReg);
										fseek(arqReg, byteOffset, SEEK_SET);
										fwrite("*",sizeof(char), 1, arqReg);
										fwrite(&valorApontado, sizeof(valorApontado), 1, arqReg);
									}
								}
								/*Caso o novo espa�o seja maior ou igual o tamanho do registro apontado pela LED,
								fa�o o novo espa�o apontar para a LED atual e a LED receber o novo valor*/
								else {
									fseek(arqReg, byteOffset, SEEK_SET);
									fwrite("*",sizeof(char), 1, arqReg);
									fwrite(&LED, sizeof(LED), 1, arqReg);
									rewind(arqReg);
									fwrite(&byteOffset, sizeof(byteOffset), 1, arqReg);
								}
	
							}
						}
					}

					//Se nao couber, insere no final
					else{
						printf("\n\nO novo registro nao cabe no espaco disponivel. Foi inserido no final do arquivo");
						fseek(arqReg, 0, SEEK_END);
						fwrite(&tamanhoRegistro, sizeof(tamanhoRegistro), 1, arqReg);
		        		fwrite(strBuffer, tamanhoRegistro, 1, arqReg);
					}

				}

				//TESTE PARA VER O VALOR DA LED
				rewind(arqReg);
		        fread(&LED, sizeof(LED), 1, arqReg);
				printf("\n\nValor novo da LED depois da manutencao: %d", LED);

				//TESTE PARA VER TODOS OS REGISTROS E O INSERIDO
				campo = 1;
			    contadorReg = 1;
		        fseek(arqReg, 2, SEEK_SET);
				rec_length = obterRegistro(arqReg, strBuffer);
				while(rec_length > 0){
					token = strtok(strBuffer, "|");
					printf("\n\nRegistro: %i", contadorReg);
					printf("  Tamanho do Registro: %i", rec_length);
					if(strstr(token, "*") != NULL){
						printf("\n\n*Registro removido anteriormente*\n");
						contadorReg++;
						rec_length = obterRegistro(arqReg, strBuffer);
						printf("\n*Tecle enter para o proximo registro*\n");
						getch();
						continue;
					}
					while(token != NULL){
						printf("\nCampo #%i: %s", campo, token);
					    token = strtok(NULL, "|");
						campo++;
					}
					printf("\n*Tecle enter para o proximo registro*");
					getch();
					contadorReg++;
					printf("\n");
					rec_length = obterRegistro(arqReg, strBuffer);
		   		}
	   		break;
	   		/* FIM INSER��O */

	   		/* REMO��O DE REGISTROS */
	   		case 3:
	   			//faz busca sequencial ate achar o ID e vai somando o valor dos registros no byteoffset
	        	matched = false;
	        	printf("\nQual o ID procurado?: ");
	        	gets(searchKey);

	        	fseek(arqReg, 2, SEEK_SET);

	        	//inicia com 4 pois tem o cabecalho + 2 do tamanho do registro ...
	        	byteOffset = 4;

	        	while(!matched && (rec_length = obterRegistro(arqReg, strBuffer)) > 0){
			        id = strtok(strBuffer, "|");
			        if(strcmp(id,searchKey) == 0){
			            matched = true;
			            continue;
			        }
			        //a cada registro o byteoffset recebe o byteoffset anterior + o tamanho do registro lido + 2 bytes do cabe�alho do que vai ser lido
			        byteOffset = byteOffset + 2 + rec_length;
			    }

			    if(matched){
			        printf("\nID localizado!");
			        printf("\nByteoffset do ID: %d", byteOffset);
			    }
			    else{
			        printf("\nO ID %s nao foi localizado!\n", searchKey);
					break;
			    }

	        	//testando o valor inicial da LED
	        	rewind(arqReg);
	        	fread(&LED, sizeof(LED), 1, arqReg);
   				printf("\nLED Atual: %d", LED);

   				if(LED == -1){
   					printf("\n\nLed estava vazia");
   					short aux;

   					//gravo *-1 no removido
   					fseek(arqReg, byteOffset, SEEK_SET);
   					fwrite("*",sizeof(char), 1, arqReg);
   					fwrite(&LED, sizeof(LED), 1, arqReg);

   					//LED recebe seu novo valor
   				 	LED = byteOffset;
   				 	rewind(arqReg);
   				 	fwrite(&LED, sizeof(LED), 1, arqReg);

					//testando o valor gravado no registro removido, posiciona depois do asterisco
					fseek(arqReg, byteOffset+1, SEEK_SET);
					fread(&aux, sizeof(aux), 1, arqReg);
					printf("\n\nByteoffset gravado no registro removido: %d", aux);

					//testando o valor final na LED
					rewind(arqReg);
	        		fread(&LED, sizeof(LED), 1, arqReg);
   					printf("\nLED Final: %d", LED);

				} else{
					//movo o ponteiro de leitura na posicao da led. Tira 2 pois quero ver o tamanho do reg
					fseek(arqReg, LED-2, SEEK_SET);

					//pega o tamanho do espa�o do q ta na led
					fread(&tamanhoLED, sizeof(tamanhoLED), 1, arqReg);

					//move o ponteiro de leitura na posicao do byteoffset procurado
					fseek(arqReg, byteOffset-2, SEEK_SET);

					//pego o tamanho do espa�o novo
					fread(&tamanhoByte, sizeof(tamanhoByte), 1, arqReg);

					//se o tamanho da led for maior que o do byte
					if(tamanhoLED > tamanhoByte){
						//verifico se o byteoffset da led aponta pra *-1. +1 pois quero pular o *
						fseek(arqReg, LED+1, SEEK_SET);
						fread(&valorApontado, sizeof(valorApontado), 1, arqReg);
						fseek(arqReg, valorApontado-2, SEEK_SET);
						fread(&tamanhoLED, sizeof(tamanhoLED), 1, arqReg);
						LEDatual = LED;
						//enquanto nao encontra um *-1 e o tamanho da LED � maior
						while(valorApontado != -1 && (tamanhoLED > tamanhoByte)){
							LEDatual = valorApontado;
							fseek(arqReg, LEDatual+1, SEEK_SET);
							fread(&valorApontado, sizeof(valorApontado), 1, arqReg);
														
							fseek(arqReg, valorApontado-2, SEEK_SET);
							fread(&tamanhoLED, sizeof(valorApontado), 1, arqReg);
						}
						
						if(valorApontado == -1){
							fseek(arqReg, LEDatual+1, SEEK_SET);
							fwrite(&byteOffset, sizeof(byteOffset), 1, arqReg);
							fseek(arqReg, byteOffset, SEEK_SET);
							fwrite("*",sizeof(char), 1, arqReg);
							fwrite(&valorApontado, sizeof(valorApontado), 1, arqReg);
						}else{
							fseek(arqReg, LEDatual, SEEK_SET);
							fwrite("*",sizeof(char), 1, arqReg);
							fwrite(&byteOffset, sizeof(byteOffset), 1, arqReg);
							fseek(arqReg, byteOffset, SEEK_SET);
							fwrite("*",sizeof(char), 1, arqReg);
							fwrite(&valorApontado, sizeof(valorApontado), 1, arqReg);
						}

					}
					//caso o novo espa�o seja maior, fa�o o novo apontar p led velha
					else {
						fseek(arqReg, byteOffset, SEEK_SET);
						fwrite("*",sizeof(char), 1, arqReg);
						fwrite(&LED, sizeof(LED), 1, arqReg);
						rewind(arqReg);
						fwrite(&byteOffset, sizeof(byteOffset), 1, arqReg);
					}

					fseek(arqReg, byteOffset+1, SEEK_SET);
					fread(&aux, sizeof(aux), 1, arqReg);
					printf("\n\nByteoffset gravado no registro removido = %d", aux);

					rewind(arqReg);
	        		fread(&LED, sizeof(LED), 1, arqReg);
   					printf("\nLED Final = %d", LED);
				}
	       	break;
	       	/* FIM REMOCAO */
	   	}

	}while(opcao < 4 && opcao > 0);

	//encerra os arquivos ao final
	fclose(arqCat);
	fclose(arqReg);
}


/*Fun��o para leitura do catalogo caractere por caractere at� terminar um campo.
  Quando encontra um caractere ';' ou '\n', retorna a quantidade de caracteres lidos.*/
int obterCampos(FILE* arqCat, char* strCampo, int* reg){
        int i = 0;
        char c;

        c = fgetc(arqCat);

        while(feof(arqCat) == 0 && (c != ';' && c != '\n')){
            strCampo[i] = c;
            i++;
            c = fgetc(arqCat);
        }

        //contador de registros encontrados
        if(c == '\n'){
            (*reg)++;
        }

		//este IF � para ele eliminar os espa�os antes de cada campo no cat�logo
        if(c == ';'){
            c = fgetc(arqCat);
        }

        strCampo[i] = '\0';
        return i;
}

/*Fun��o para leitura do tamanho de um registro no arquivo de registros.
  Faz a leitura da quantidade de caracteres do registro e armazena estes em um buffer.
  Retorna o tamanho do registro lido*/
short obterRegistro(FILE *arqReg, char *strBuffer){
        short rec_length;

        if (fread(&rec_length, sizeof(rec_length), 1, arqReg) == 0){
            return 0;
        }
        rec_length = fread(strBuffer, 1 ,rec_length, arqReg);

        strBuffer[rec_length] = '\0';
        return rec_length;
}

/*Fun��o que recebe os dados do novo registro e armazena no buffer recebido*/
void receberDados(char* strBuffer){
	char strCampo[256];
	strBuffer[0] = '\0';
	printf("\n*Insira os dados do novo Registro*");
	printf("\nID do Aluno: ");
    gets(strCampo);
    strcat(strBuffer,strCampo);
    strcat(strBuffer,"|");

    printf("Nome do Autor: ");
    gets(strCampo);
    strcat(strBuffer,strCampo);
    strcat(strBuffer,"|");

    printf("Titulo do Trabalho: ");
    gets(strCampo);
    strcat(strBuffer,strCampo);
    strcat(strBuffer,"|");

    printf("Nome do Curso: ");
    gets(strCampo);
    strcat(strBuffer,strCampo);
    strcat(strBuffer,"|");

    printf("Tipo do Trabalho: ");
    gets(strCampo);
    strcat(strBuffer,strCampo);
    strcat(strBuffer,"|");
}


