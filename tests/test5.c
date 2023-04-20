
int pgcd(int a, int b){
    while (!(a*b == 0)){
        if(a>b) a = a-b; 
        else b = b-a;
    }
    if (a == 0) return b;
	else return a;
}