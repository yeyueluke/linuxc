int main() { 
    char* str = new char[8];
	for(int i=0;i<8;i++){ 
	    str[i] = (char)i; 
	} 
	delete[] str;
	return 0;
} 
