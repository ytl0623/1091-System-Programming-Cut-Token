# include <cctype>
# include <cstdio>
# include <stdio.h>
# include <cstdlib>
# include <stdlib.h>
# include <cstring>
# include <string>
# include <string.h>
# include <vector>
# include <sstream>
# include <fstream>
# include <iostream>
# include <algorithm>

using namespace std ;

static ifstream inFile ;
static ofstream outFile ;

typedef struct TokenData {
  int type ;
  int number ;
  string token ;
} TokenData ;

class LexicialAnalysis {
public:
  vector<TokenData> opCodeTable ;
  vector<TokenData> pseudoTable ;
  vector<TokenData> registerTable ;
  vector<TokenData> delimiterTable ;
  vector<TokenData> labelTable ;
  vector<TokenData> integerTable ;
  vector<TokenData> stringTable ;

  void Run() {
    CutToken() ;
  }

  void StringToUpper( string & str ) {
    for( int i = 0 ; i < str.size() ; i++ ) {
      str[i] = toupper( str[i] ) ;
    }
  }

  void InputTable() {
    int count = 0 ;     // index of table
    string tableName = "\0" ;
    TokenData tempToken ;

    for( int i = 1 ; i <= 4 ; i++ ) {
      count = 0 ;

      if( i == 1 ) {
        tableName = "Table1.table" ;
        opCodeTable.push_back(tempToken) ;     // index[0]
      }
      else if( i == 2 ) {
        tableName = "Table2.table" ;
        pseudoTable.push_back(tempToken) ;     // index[0]
      }
      else if( i == 3 ) {
        tableName = "Table3.table" ;
        registerTable.push_back(tempToken) ;     // index[0]
      }
      else if( i == 4 ) {
        tableName = "Table4.table" ;
        delimiterTable.push_back(tempToken) ;     // index[0]
      }

      inFile.open( tableName.c_str(), fstream::in ) ;

      while( !inFile.eof() ) {
        count++ ;

        inFile >> tempToken.token ;
        //cout << tempToken.token << " / " ;
        StringToUpper( tempToken.token ) ;
        //cout << tempToken.token << endl ;
        tempToken.type = i ;
        tempToken.number = count ;

        if( i == 1 ) {
          opCodeTable.push_back(tempToken) ;
        }
        else if( i == 2 ) {
          pseudoTable.push_back(tempToken) ;
        }
        else if( i == 3 ) {
          registerTable.push_back(tempToken) ;
        }
        else if( i == 4 ) {
          delimiterTable.push_back(tempToken) ;
        }
      }
      inFile.close() ;
    }
  }

private:
  bool Isdelimiter( char ch, TokenData& delimiterToken ) {
    string check( 1, ch ) ;

    for( int i = 0 ; i < delimiterTable.size() ; i++ ) {
      if( check == delimiterTable[i].token ) {
        delimiterToken = delimiterTable[i] ;
        return true ;
      }
    }
    return false ;
  }

  void CutToken() {
    string x86code = "\0" ;
    string singleToken = "\0" ;
    TokenData delimiterToken ;

    while( getline( inFile, x86code ) ) {     // '\n' NOT included
      outFile << x86code << endl ;

      for( int i = 0 ; i < x86code.length() ; i++ ) {
        if( Isdelimiter( x86code[i], delimiterToken ) || ( x86code[i] == ' ' ) || ( x86code[i] == '\t' ) || ( x86code[i] == '\n' ) ) {     // NOT character

          if( !singleToken.empty() ) {     // cut token by white space
            Processing( singleToken, x86code[i] ) ;
            singleToken.clear() ;
          }

          if( Isdelimiter( x86code[i], delimiterToken ) ) {     // table4: delimiter
            outFile << "(" << delimiterToken.type << "," << delimiterToken.number << ")" ;
            
            if( x86code[i] == ';' ) {     // comment
              break ;
            }
          }
        }
        else {
          singleToken.push_back( x86code[i] ) ;

          if( i == x86code.length() - 1 && !singleToken.empty() ) {     // end of line
            Processing( singleToken, x86code[i] ) ;
            singleToken.clear() ;
          }
        }
      }
      
      outFile << endl ;
      x86code.clear() ;
    }
  }

  void Processing( string singleToken, char currentChar ) {
    string save = singleToken ;
    int ASCII = 0 ;
    int index = 0 ;
      
    if( currentChar == '\'' ) {     // e.g.: 'Green'
      stringTable.resize( 100 ) ;
      index = 0 ;
      TokenData stringToken ;
        
      for( int i = 0 ; i < singleToken.size() ; i++ ) {     // e.g.: 'a' != 'A'
        ASCII = ASCII + (int)singleToken[i] ;
      }
    
      while( stringTable[ASCII % 100 + index].token != "\0" && stringTable[ASCII % 100 + index].token != singleToken ) {
        index = index + 1 ;
      
        if( ( ASCII % 100 + index ) > 99 ) {
          index = -( ASCII % 100 ) ;
        }
      }
      
      //cout << singleToken << "/" << index << "/" << ASCII % 100 + index << endl ;
      stringToken.type = 7 ;
      stringToken.number = ASCII % 100 + index ;
      stringToken.token = singleToken ;
      stringTable[ASCII % 100 + index] = stringToken;
      
      outFile << "(" << stringToken.type << "," << stringToken.number << ")" ;
      return ;     // table7: string
    }
    else if( singleToken[singleToken.size() - 1] == 'H' && singleToken != "AH" && singleToken != "BH" && singleToken != "CH" && singleToken != "DH" ) {     // e.g.: 1234H
      integerTable.resize( 100 ) ;
      index = 0 ;
      int i = 0 ;
        
      for( i = 0 ; isxdigit( singleToken[i] ) ; i++ ) {     // hex are any of: 0 1 2 3 4 5 6 7 8 9 a b c d e f A B C D E F
        singleToken[i] = toupper( singleToken[i] ) ;
        ASCII = ASCII + (int)singleToken[i] ;
      }
      ASCII = ASCII + 72 ;     // 'H'
      
      if( i == singleToken.size() - 1 ) {     // if 'H' is the last character of singleToken, put singleToken into integerTable.
        TokenData intToken ;
        
        while( integerTable[ASCII % 100 + index].token != "\0" && integerTable[ASCII % 100 + index].token != singleToken ) {
          index = index + 1 ;
          if ( ( ASCII % 100 + index ) > 99 ) {
            index = -( ASCII % 100 ) ;
          }
        }
        
        intToken.type = 6 ;
        intToken.number = ASCII % 100 + index ;
        intToken.token = singleToken ;
        integerTable[ASCII % 100 + index] = intToken ;
        outFile << "(" << intToken.type << "," << intToken.number << ")" ;
        return ;     // table6: integer
      }
    }
      
    for( int i = 0 ; i < singleToken.size() ; i++ ) {     // e.g. a->A, cuz ADD = add
      singleToken[i] = toupper( singleToken[i] ) ;
    }
    
    for( int i = 0 ; i < opCodeTable.size() ; i++ ) {     // table1: opcode
      if( singleToken == opCodeTable[i].token ) {
        outFile << "(" << opCodeTable[i].type << "," << opCodeTable[i].number << ")" ;
        return ;
      }
    }
    
    for( int i = 0 ; i < pseudoTable.size() ; i++ ) {     // table2: pseudo
      if( singleToken == pseudoTable[i].token ) {
        outFile << "(" << pseudoTable[i].type << "," << pseudoTable[i].number << ")" ;
        return ;
      }
    }
    
    for( int i = 0 ; i < registerTable.size() ; i++ ) {    // table3: register
      if( singleToken == registerTable[i].token ) {
        outFile << "(" << registerTable[i].type << "," << registerTable[i].number << ")" ;
        return ;
      }
    }
    
    ASCII = 0 ;
    singleToken = save ;
    labelTable.resize( 100 ) ;
    index = 0;
    TokenData labelToken ;
    
    for( int i = 0 ; i < singleToken.size() ; i++ ) {
      ASCII = ASCII + (int)singleToken[i] ;
    }
    
    while( labelTable[ASCII % 100 + index].token != "\0" && labelTable[ASCII % 100 + index].token != singleToken ) {     // table5: label
      index = index + 1 ;
        
      if( ( ASCII % 100 + index ) > 99 ) {
        index = -( ASCII % 100 ) ;
      }         
    }
    
    labelToken.type = 5 ;
    labelToken.number = ASCII % 100 + index ;
    labelToken.token = singleToken ;
    labelTable[ASCII % 100 + index] = labelToken ;
    outFile << "(" << labelToken.type << "," << labelToken.number << ")" ;
    return ;
  }
};

int main() {
  string command = "\0" ;
  string fileNum = "\0" ;

  LexicialAnalysis cutToken ;
  cutToken.InputTable() ;

  while(1) {
    cout << "*****************************************" << endl ;
    cout << "***** Lexical Analysis              *****" << endl ;
    cout << "***** 0 : Quit                      *****" << endl ;
    cout << "***** 1 : Cut Token                 *****" << endl ;
    cout << "*****************************************" << endl ;
    cout << "Input a command(0, 1): " ;
    cin >> command ;

    if( command == "0" ) {
      return 0 ;     // the only reason to quit while
    }
    else if( command == "1" ) {
      bool stop = false ;     // 0 to quit

      while( !stop ) {
        cout << endl << "Input a file number ([0] to quit): " ;
        cin >> fileNum ;

        if( fileNum == "0" ) {
          cout << endl ;
          stop = true ;
        }
        else {
          string inFileName = fileNum + ".txt" ;
          inFile.open( inFileName.c_str(), fstream::in ) ;

          if( inFile.is_open() ) {
            string outFileName = fileNum + "'s output.txt" ;
            outFile.open( outFileName.c_str(), fstream::out ) ;

            cutToken.Run() ;

            stop = true ;
            cout << endl ;
          }
          else {
            cout << endl << inFileName << " does not exist!" << endl ;
          }
          
          inFile.close() ;
          outFile.close() ;
        }
      }
    }
    else {
      cout << endl << "Command does not exit!" << endl << endl ;
    }
  }
}
