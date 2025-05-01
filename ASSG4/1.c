#include <stdio.h>

int mod(int x){
	return x < 0 ? -x : x;
}

struct cord{
	int x;
	int y;
};

typedef struct cord cord;

float triangleArea(cord s1,cord s2,cord s3){
	float t = (float) (mod(s1.x*(s2.y-s3.y)+ s2.x*(s3.y-s1.y) +s3.x*(s1.y-s2.y)))*0.5 ;
	return t;
}

int main(){
	cord s1,s2,s3;
	scanf("%d %d",&(s1.x),&(s1.y));
	scanf("%d %d",&(s2.x),&(s2.y));
	scanf("%d %d",&(s3.x),&(s3.y));
	float area = triangleArea(s1,s2,s3);
	printf("Area is: %0.2f\n",area);
	return 0;
	
	/*
	s1.x=1;
	s1.y=2;
	
	s2.x=4;
	s2.y=2;
	
	s3.x=3;
	s3.y=5;
	float area = triangleArea(s1,s2,s3);
	printf("Area is: %0.2f\n",area);
	//printf("%d",(s1.x*(s2.y-s3.y))+ (s2.x*(s3.y-s1.y)) +(s3.x*(s1.y-s2.y)));
	*/
	
}
