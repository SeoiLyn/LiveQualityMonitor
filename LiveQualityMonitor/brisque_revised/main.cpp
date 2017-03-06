#include "brisque.h"

//rescaling based on training data i libsvm
float rescale_vector[36][2];

int find_option( int argc, char **argv, const char *option )
{
    for( int i = 1; i < argc; i++ )
        if( strcmp( argv[i], option ) == 0 )
            return i;
    return -1;
}

int read_int( int argc, char **argv, const char *option, int default_value )
{
    int iplace = find_option( argc, argv, option );
    if( iplace >= 0 && iplace < argc-1 )
        return atoi( argv[iplace+1] );
    return default_value;
}

char *read_string( int argc, char **argv, const char *option, char *default_value )
{
    int iplace = find_option( argc, argv, option );
    if( iplace >= 0 && iplace < argc-1 )
        return argv[iplace+1];
    return default_value;
}

int read_range_file(string prefix) {
	//check if file exists
	char buff[100];
	int i;
	string range_fname = prefix + "allrange";
	FILE* range_file = fopen(range_fname.c_str(), "r");
	if(range_file == NULL) return 1;
	//assume standard file format for this program	
	fgets(buff, 100, range_file);
	fgets(buff, 100, range_file);
	//now we can fill the array
	for(i = 0; i < 36; ++i) {
		float a, b, c; 
	    fscanf(range_file, "%f %f %f", &a, &b, &c);
		rescale_vector[i][0] = b;
		rescale_vector[i][1] = c;
	}
	return 0;	
}


int  main(int argc, char** argv)
{
    if( find_option( argc, argv, "-h" ) >= 0 || argc <= 2 )
    {
        printf( "Options:\n" );
        printf( "-t <int> to specify if you want to retrain \n" );
        printf( "-im <filename> to specify the image file  name\n" );
        return 0;
    }

    string prefix = argv[0];
    prefix = prefix.substr(0,prefix.find_last_of("/\\")+1);

  //read in the allrange file to setup internal scaling array
    if(read_range_file(prefix)) {
		cerr<<"unable to open allrange file"<<endl;
		return -1;
    }
   
  
  int istrain    = read_int( argc, argv, "-t",1 );
  char *filename = read_string( argc, argv, "-im", NULL );

  float qualityscore;

  if(!istrain) //default value is 1 for false?
   trainModel();
  
  qualityscore = computescore(prefix, filename);
  cout<<"score in main file is given by:"<<qualityscore<<endl;
}
