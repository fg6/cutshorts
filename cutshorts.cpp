#include <zlib.h>
//#include <stdio.h>
#include "kseq.h"
#include <iostream>
#include <iomanip>  //setprecision
#include <fstream>

static gzFile fp;
static  FILE * outF;

KSEQ_INIT(gzFile, gzread)
int readseqs(int minlength);
int calc(void);
int myread(char* file, int minlength);

int main(int argc, char *argv[])
{ 

  if (argc == 2) {
   fprintf(stderr, "Usage: %s <reads.fq/fa> %s minLenght \n", argv[0],argv[1]);
   return 1;
  }	
  if((fp = gzopen(argv[1],"r")) == NULL){ 
    printf("ERROR main:: missing input file  !! \n");
    return 1;
  }
  std::string  minL=  argv[2];
  int ml= atoi(argv[2]);

  size_t pos = 0;
  std::string myname = argv[1];
  std::string token;
  std::string delimiter = "/";
  while ((pos = myname.find(delimiter)) != std::string::npos) {
    token = myname.substr(0, pos);
    myname.erase(0, pos + delimiter.length());
  }
  myname= "min" + minL + "_" + myname;
  std::cout<<"Removing sequences shorter than " << minL << " bp   output: " <<  myname << std::endl;
  outF = fopen (myname.c_str(),"w");


  int mlines=myread(argv[1],ml); //does not work with seq on multi-lines! but faster
  //int mlines=1;
  if(mlines)
  	readseqs(ml);
  
  fclose(outF);

  return 0;
}


// ---------------------------------------- //
int readseqs(int minlength){
// ---------------------------------------- //

  kseq_t *seq;
  int l=-1;
  seq = kseq_init(fp);
  char out[5]={">"};
  char fq[5]={"@"};

   


  if(1)std::cout << " ...using kseq to read file" << std::endl;
  int tot=0;
  int eli=0;
  while ((l = kseq_read(seq)) >= 0){
	tot++;    
	if(seq->seq.l >= minlength){
	        if(seq->qual.l) out[0]=fq[0];

		fprintf(outF,"%c%s %s\n", out[0],seq->name.s, seq->comment.s);  
        	fprintf(outF,"%s\n", seq->seq.s);  
		if(seq->qual.l)fprintf(outF,"+\n%s\n", seq->qual.s);
	}else{eli++;}
  }
  kseq_destroy(seq);
  gzclose(fp);
 

 std::cout << "Total sequences: " << tot << " eliminated sequences: " << eli
        <<  std::fixed << std::setprecision(1) << "  (" << eli*100./tot <<"%)"<< std::endl;
 
 return 0;
}

// ---------------------------------------- //
int myread(char* file, int minlength)
// ---------------------------------------- //
{ // won't work with seq on multilines!
  char fq[5]={"@"};
  char fa[5]={">"};
  char plus[5]={"+"};
//  char out[5]={">"};
  int readevery=1;
  int pri=0;
  std::ifstream infile(file); 

  std::string line; 
  getline(infile,line);  // first line
  if(line.at(0)==fq[0]) {
	readevery=4;  // fastq input file
	//out[0]=fq[0];
  }
  else if(line.at(0)==fa[0]) readevery=2;  // fasta input file
  else {
		std::cout << " Error: cannot determine if input file is fasta or fastq " << std::endl;
		return(1);
	}
  if(pri)std::cout<< line<< std::endl; 
  getline(infile,line);  //sec line
  if(pri)std::cout<< line<< std::endl;
  getline(infile,line);  //third line
  if(pri)std::cout<< line<< std::endl;
 
  
  if(line.at(0)!=plus[0] && line.at(0)!=fa[0]){ 
	if(pri)std::cout<< "Sequences on multiple lines..." << line.at(0)<< std::endl;  // seq on single line 
	return(1);
  }else{
        if(pri)std::cout<< "Sequences on single line " << std::endl;
  }


  int eli=0;
  int nseq=0;

  infile.clear();  // start over
  infile.seekg (0, std::ios::beg);

  while (!infile.eof()){
    nseq++;
    std::string name;
    std::string seq;
    std::string qual;
    std::string p;

    getline(infile,name);
    getline(infile,seq);
    if(readevery==4) { //for fastq
          getline(infile,p);
          getline(infile,qual);
    }


//    if(nseq<10)std::cout << nseq << " " << name << " " << seq.length() << " " << out[0] << std::endl;

    if(seq.length() >= minlength){
      fprintf(outF,"%s\n",name.c_str());    // "%c%s\n",out[0],(name.erase(0,1)).c_str());
      fprintf(outF,"%s\n", seq.c_str());
      if(readevery==4) { //for fastq
          fprintf(outF,"+\n%s\n", qual.c_str());
      }
   }else{eli++;}

 }
 std::cout << "Total sequences: " << nseq-1 << " eliminated sequences: " << eli-1 
	<<  std::fixed << std::setprecision(1) << "  (" << (eli-1)*100./nseq <<"%)"<< std::endl;
 return 0;
}

