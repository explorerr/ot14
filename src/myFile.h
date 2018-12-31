/*
 * myFile.h
 *
 *  Created on: Mar 19, 2009
 *      Author: zrui
 */

#ifndef MYFILE_H_
#define MYFILE_H_
#include "Exceptions.h"
#include "lib.h"
#include "stdlib.h"
#include "string.h"
//#include <vector>
#include <string>
using namespace std;
class myFile {
public:


	/**
	  * a class designed to load the file into vector of strings
	  * then will provide line by line query or token by token query
	  * will keep a pointer points to the curLine
	  * curLine will be NULL pointer if the file is empty
	  *                              if there is no more line in the file
	  * 
	  */



	myFile(char* fName, char* extention){

		curLine = NULL;
		fullFilePath = NULL;
		getFullFilePath(&fullFilePath, fName, extention);
		file = fopen(fullFilePath, "r");

		if(file)
			printf("start to read file <%s>...\n", fName);
		else {
			printf("error reading file <%s>...\n", fullFilePath);
			throw StoppingException("Error: File open failed.\n");
		}
		char* temp = NULL;
		if( ! fgetline_123(&temp, file) ){
			printf("File empty\n");
			curLineId = -1;
			curLine = NULL;
			tokenized = false;
			tokens = NULL;
			curToken = NULL;
			curTokenId = -1;
			fclose(file);
			return;
		}
		else{
			curLine = temp;
			curLineId = 0;
		}

		tokenized = false;
		tokens = NULL;
		curToken = NULL;
		curTokenId = -1;
		delis = (char*) malloc(sizeof(char)*2);
		strcpy(delis," ");
		delis[1]= '\0';


	};

	virtual ~myFile(){
		if( curLine != NULL)
			free(curLine);
		free(delis);
	};

	/**
	 * This function will try to get the next line of the file
	 * If there is no mroe line, the function will return NULL pointer
	 * the curLine will also be set as NULL, and other status of the file will also be set at NULl
	 *
	 * Else if there are lines, then the status will be set accordingly
	 */
	char* getNxtLine(){


		char* temp;
		if( ! fgetline_123(&temp, file)) {
			free(curLine);
			curLine = NULL;
			curLineId = -1;

			if(tokenized){
				freeTokenArray(tokens, curToCnt);
				tokens = NULL;
				curTokenId = -1;
				curToCnt = 0;
			}

		}
		else{
			free(curLine);
			curLine = temp;
			curLineId++;

			if(tokenized){
				freeTokenArray(tokens, curToCnt);
				tokens = NULL;
				curToCnt = 0;
				tokenizeCurLine();
			}

		}

		return curLine;

	};

	bool getNxtLineAndTokenize(){
		if(!getNxtLine())
			return false;

		freeTokenArray(tokens, curToCnt);
		tokens = NULL;
		curToCnt = 0;
		return tokenizeCurLine();
	}


	/**
	  * This function will try to find the next token till the end of the file
	  * Thus this function will also change the curLine pointer
	  * if there is no more tokens in the current line
	  * then the function will keep searching (incrementing pointer to current line)
	  * untill it hits a non-NULL line and token
	  */

	char* getNxtToken(){
		if(!tokenized){
			printf("Error: file lines are not tokenized.\n");
			throw StoppingException("From myFile::getNxtToken.\n");
		}

		curTokenId++;
		while( curTokenId == -1 || curTokenId >= curToCnt ){
			getNxtLine();

			freeTokenArray(tokens, curToCnt);
			if(!curLine){
				curToken = NULL;
				curTokenId = -1;
				return NULL;
			}

			tokenizeCurLine();
			
		}
		return curToken = tokens[curTokenId];
	}

	/**
	  * this function does no more than setting the delis for the tokenzier 
	  *                                 and setting tokenized sign to true 
	  * 
      * 
	  * will return the result of tokenizeCurLine()
	  */
	
	bool tokenize(char* delis){

		if(this->delis)
			free(this->delis);
		tokenized = true;
		this->delis = (char*) malloc( sizeof(char) * (strlen(delis)+1) );
		strcpy(this->delis, delis);

		return tokenizeCurLine();

	}


	/**
	  * This function will first check if the pointer to curLine is NULL
	  * if the curLine is NULL then return false
	  * assumptions are: the pointer to curLine points to the curLine of the file
	  *                                                     (if there is any)
	  * if the tokenization is seccessful then return true
	  * otherwise return false
	  */

	bool tokenizeCurLine(){
		if(!curLine)
			return false;
		tokens = split(curLine, curToCnt, delis);
		curToken = (tokens) ? tokens[0] : NULL;
		curTokenId = (tokens) ? 0 : -1;
		if(tokens)
			return true;
		return false;
	}


	void gotoLastLine(){



		char* lastLineRev = NULL;

		int cnt = 0;
		fpos_t position = ftell(file);
		fseek(file, -1, SEEK_END);

		position = ftell(file);
		position = position - sizeof(char);
		char cur = getc(file);

		while(cur=='\n' || cur=='\t' || cur==' '){
			position = position - sizeof(char);
			fsetpos (file, &position);
			cur = getc(file);
		}
		while( cur != '\n' ) {
			lastLineRev = (char*) realloc( lastLineRev, sizeof(char) * ((++cnt)+1) );

			lastLineRev[cnt-1] = cur;
			lastLineRev[cnt]= '\0';
			position = position - sizeof(char);
			fsetpos (file, &position);

			cur = getc(file);
		}

		if( lastLineRev ) {

			if(curLine)
				free(curLine);

			curLine = (char*) malloc( sizeof(char) * (cnt+1) );
			for(int i=0; i<cnt; i++)
				curLine[i] = lastLineRev[cnt-i-1];
			curLine[cnt] = '\0';
			curLineId = 0;

			free(lastLineRev);
		}
			// else: there is no previous line, means the file is empty then the current status file is correct

		if(tokenized){
			tokenizeCurLine();
		}


	}


	void rewand(){

		clearStatus();
		fseek ( file , 0 , SEEK_SET );

		if(!file){
			printf("error rewinding file!\n");
			free(fullFilePath);
			throw StoppingException("Error: File reading failed.\n");
		}
		char* temp = NULL;
		if( ! fgetline_123(&temp, file) ){
			printf("File empty\n");
			curLineId = -1;
			curLine = NULL;
			tokenized = false;
			tokens = NULL;
			curToken = NULL;
			curTokenId = -1;
			free(fullFilePath);
			fclose(file);
			return;
		}
		else{
			curLine = temp;
			curLineId = 0;
		}
		if(tokenized){
			tokenizeCurLine();
		}



	}

	void clearStatus(){

		if(curLine)
			free(curLine);
		curLineId = -1;

		if(curToken)
			free(curToken);
		curTokenId = -1;
		curToCnt = 0;



	}

	char*		curLine;
	char*		curToken;
	__int64		curLineId;
	bool		tokenized;
	__int64		curTokenId;
	__int64		curToCnt;

private:
	FILE* file;
	char** tokens;
	char* delis;
	char* fullFilePath;


};

#endif /* MYFILE_H_ */
