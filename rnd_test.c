#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <assert.h>

float rand_normal(float mean, float stddev)
{//Box muller method
    static float n2 = 0.0;
    static int n2_cached = 0;
    if (!n2_cached)
    {
        float x, y, r;
        do
        {
            x = 2.0*rand()/RAND_MAX - 1;
            y = 2.0*rand()/RAND_MAX - 1;

            r = x*x + y*y;
        }
        while (r == 0.0 || r > 1.0);
        {
            float d = sqrt(-2.0*log(r)/r);
            float n1 = x*d;
            n2 = y*d;
            float result = n1*stddev + mean;
            n2_cached = 1;
            return result;
        }
    }
    else
    {
        n2_cached = 0;
        return n2*stddev + mean;
    }
}

float uniform(float xMin, float xMax){
    float x = ((float) random()/RAND_MAX);
    return xMin + ( xMax - xMin ) * x;
}

float triangular( float xMin, float xMax, float c ){
    assert( xMin < xMax && xMin <= c && c <= xMax );
    float p = uniform(0.0, 1.0);
    float q = 1-p;
    if( p <= ( c - xMin ) / ( xMax - xMin ) )
        return (xMin + sqrt( ( xMax - xMin ) * ( c - xMin ) * p ));
    else
        return (xMax - sqrt( ( xMax - xMin ) * ( xMax - c ) * q ));
}

float bimodal( float low1, float high1, float mode1, float low2, float high2, float mode2 ){
    if(((float) random()/RAND_MAX) < 0.5)
        return triangular( low1, high1, mode1 );
    else
        return triangular( low2, high2, mode2 );
}

int main(){
    srand(time(NULL));
    int i;
    float a;
    for(i=0;i<400;i++){
        a = rand_normal(1,1);
        printf("a = %f\n", a);
    }
    return 0;

}
