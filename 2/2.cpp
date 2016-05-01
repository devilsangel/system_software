#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <list>
#include <cstdlib>
#include <iomanip>
using namespace std;
map <string,string> optab;
map <string,string> symtab;
fstream fout,fin;
void opinit(){
	fstream f("optab");
	string op;
	while(f){f>>op;f>>optab[op];}
}
class Line
{
public:
	string addr,label,opcode,operand;
	Line(string line,int i=0){
		stringstream s(line);
		char c;
		if(i){
			s>>addr;
			s.get(c);
		}
		s.get(c);
		if(c!='\t'){
			s.unget();
			s>>label>>opcode>>operand;
		}else
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
	fout.open("inter");
	fin.open("sic");
	string line,name;
	int locctr,stradr;
	while(getline(fin,line,'\n')){
		Line l(line);
		if(l.opcode=="END"){
			fout<<decToHex(locctr,4)<<'\t'<<l.label<<'\t'<<l.opcode<<'\t'<<l.operand<<endl;
			break;
		}
		if(l.opcode=="START"){
			stringstream s(l.operand);
			s>>hex>>locctr;
			stradr=locctr;
			name=l.label;
			fout<<decToHex(locctr,4)<<'\t'<<l.label<<'\t'<<l.opcode<<'\t'<<l.operand<<endl;
			continue;
		}
		fout<<decToHex(locctr,4)<<'\t'<<l.label<<'\t'<<l.opcode<<'\t'<<l.operand<<endl;
		if(l.label!=""){
			if(symtab.find(l.label)==symtab.end())
				symtab[l.label]=decToHex(locctr,4);
			else{
				cout<<"error: duplicate label "<<l.label<<endl;
				return 0;
			}
		}
		if(optab.find(l.opcode)!=optab.end())
			locctr+=3;
		else if(l.opcode=="WORD")
			locctr+=3;
		else if(l.opcode=="BYTE"){
			if(l.operand[0]=='X')
				locctr+=(l.operand.length()-3)/2;
			else
				locctr+=l.operand.length()-3;
		}
		else if(l.opcode=="RESW")
			locctr+=3*atoi(l.operand.c_str());
		else if(l.opcode=="RESB")
			locctr+=atoi(l.operand.c_str());
		else{
			cout<<"error: invalid opcode "<<l.opcode<<endl;
			return 0;
		}
	}
	fin.close();
	fin.open("inter");
	fout.close();
	fout.open("out");
	int ret=0,tsize=0;
	bool rec=1,n=1;
	fout<<"H^"<<decToHex(stradr,6)<<"^"<<decToHex(locctr-stradr,6)<<endl;
	while(getline(fin,line,'\n')){
		Line l(line,1);
		rec=1;
		if(l.opcode=="END")
			break;
		if(l.opcode=="START")
			continue;
		string opVal="",symVal="";
		if(optab.find(l.opcode)!=optab.end()){
			opVal=optab[l.opcode];
			if(symtab.find(l.operand)!=symtab.end())
				symVal=symtab[l.operand];
			else{
				cout<<"error: undefinded operand "<<l.operand<<endl;
				return 0;
			}
		}else if(l.opcode=="WORD"){
			symVal=decToHex(atoi(l.operand.c_str()),6);
		}else if(l.opcode=="BYTE"){
			if(l.operand[0]=='X')
				symVal=l.operand.substr(2,l.operand.length()-3);
			else
				for(int i=2;i<l.operand.length()-1;i++)
					symVal+=decToHex(l.operand[i],2);
		}else if(l.opcode=="RESW"||l.opcode=="RESB"){
			createNew(ret,tsize);
			fout<<endl;
			n=1;
			rec=0;
		}
		if(rec){
			if(n){
				fout<<"T^00"<<l.addr<<"^  ";
				tsize=0;
				n=0;
				ret=-2;
			}
			if(tsize+(opVal.length()+symVal.length())/2>30){
				createNew(ret,tsize);
				fout<<endl<<"T^"<<"00"<<l.addr<<"^  ";
				tsize=0;
				ret=-2;
			}
			tsize+=(opVal.length()+symVal.length())/2;
			ret=ret-1-opVal.length()-symVal.length();
			fout<<"^"<<opVal<<symVal;
		}
	}
	createNew(ret,tsize);
	if(n)
		fout<<"E^"<<decToHex(stradr,6);
	else
		fout<<endl<<"E^"<<decToHex(stradr,6);
}