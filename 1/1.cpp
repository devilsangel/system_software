#include <iostream>
#include <fstream>
#include <map>
#include <iomanip>
#include <list>
#include <sstream>
#include <cstdlib>
using namespace std;
map <string,string> optab;
map <string,pair<string,list<int>>> symtab;
fstream fout("out");
void opinit(){
	fstream f("optab");
	string line;
	while(getline(f,line,'\n')){
		stringstream s(line);
		string op;
		s>>op;
		s>>optab[op];
	}
}
class Line
{
public:
	string label,opcode,operand;
	Line(string line){
		stringstream s(line);
		char c;
		s.get(c);
		if(c!='\t'){
			s.unget();
			s>>label>>opcode>>operand;
		}
		else
			s>>opcode>>operand;
	};
};
string decToHex(int i,int j){
	stringstream s;
	s<<setfill('0')<<setw(j)<<hex<<uppercase<<i;
	return s.str();
}
void createNew(int ret,int tsize){
	fout.seekg(ret,fout.cur);
	fout<<decToHex(tsize,2);
	fout.seekg(0,fout.end);
}
int main(){
	opinit();
	fstream f("sic");
	string line;
	int locctr,size,stradr,ret=0,tsize=0,len=0;
	bool n=0;
	string name;
	while(getline(f,line,'\n')){
		len=0;
		Line l(line);
		if(l.opcode=="END")
			break;
		if(l.opcode=="START"){
			stringstream s(l.operand);
			s>>hex>>locctr;
			stradr=locctr;
			name=l.label;
			fout<<"H^"<<name<<"^"<<decToHex(locctr,6)<<endl<<"T^"<<decToHex(locctr,6)<<"^  ";
			ret=-2;
			continue;
		}
		if(l.label!=""){
			if(symtab.find(l.label)==symtab.end()){
				pair<string,list<int>> p;
				p.first=decToHex(locctr,4);
				symtab[l.label]=p;
			}else{
				symtab[l.label].first=decToHex(locctr,4);
				if(!n)
					createNew(ret,tsize);
				for(auto iter=symtab[l.label].second.begin();iter!=symtab[l.label].second.end();iter++){
					fout<<endl<<"T^"<<decToHex(*iter+1,6)<<"^02^"<<decToHex(locctr,4);
				}
				tsize=2;
				ret=-7;
				n=1;
			}
		}
		string symVal="",opVal="";
		if(optab.find(l.opcode)!=optab.end()){
			opVal=optab[l.opcode];
			if(symtab.find(l.operand)!=symtab.end()){
				if(symtab[l.operand].first!="")
					symVal=symtab[l.operand].first;
				else{
					symVal="0000";
					symtab[l.operand].second.push_back(locctr);
				}
			}
			else{
				pair<string,list<int>> p;
				p.second.push_back(locctr);
				symtab[l.operand]=p;
				symVal="0000";
			}
			len=3;
			locctr+=3;
		}else if(l.opcode=="WORD"){
			locctr+=3;
			len=3;
			symVal=decToHex(atoi(l.operand.c_str()),6);
		}else if(l.opcode=="BYTE"){
			if(l.operand[0]=='X'){
				len=(l.operand.length()-3)/2;
				symVal=l.operand.substr(2,l.operand.length()-3);
			}
			else{
				len=l.operand.length()-3;
				for(int i=2;i<l.operand.length()-1;i++)
					symVal+=decToHex(l.operand[i],2);
			}
			locctr+=len;
		}else if(l.opcode=="RESW"){
			locctr+=3*atoi(l.operand.c_str());
		}else if(l.opcode=="RESB"){
			locctr+=atoi(l.operand.c_str());
		}else{
			cout<<"error: "<<l.opcode<<endl;
			return 0;
		}
		if(len){
			if(n){
				fout<<endl<<"T^"<<decToHex(locctr-len,6)<<"^  ";
				tsize=0;
				ret=-2;
				n=0;
			}
			if(tsize+len>30){
				createNew(ret,tsize);
				fout<<endl<<"T^"<<decToHex(locctr-len,6)<<"^  ";
				tsize=0;
				ret=-2;
			}
			ret=ret-opVal.length()-symVal.length()-1;
			tsize+=len;
			fout<<"^"<<opVal<<symVal;
		}
	}
	createNew(ret,tsize);
	fout.seekg(2+name.length()+7,fout.beg);
	fout<<"^"<<decToHex(locctr-stradr,6)<<endl<<"T";	
}