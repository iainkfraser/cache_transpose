/*
* (C) Iain Fraser
* Excercise in cache aware and cache oblivous algorithms. Simplify problem
* by only dealing with square matrices and out situ.
*/

#include <stdio.h>
#include <assert.h>
#include <sys/time.h>
#include <stdint.h>

#if 1 
#define N	1024	
#define L	( 64 / sizeof(int) )	// cat /sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size	
#else
#define N	8
#define L	4
#endif


typedef int matrix[N][N];

/* not cache efficent :) */
void print( matrix m ){
	for( int i = 0; i < N; i++ ){
		for( int j = 0; j < N; j++ ){
			printf("%d ", m[i][j] );
		}

		printf("\n");
	}
} 

void init( matrix m ){
	for( int i = 0; i< N; i++ ){
		for( int j = 0; j < N; j++ ){
			m[i][j] = (N*i) + ( j + 1 );	
		}
	}
}

// cache ignorant implementation.
void naive( matrix in, matrix out ){

	for( int i = 0; i < N; i++ ){
		for( int j = 0; j < N; j++ ){
			out[j][i] = in[i][j];
		}
	}
}

// cache aware.
void caware( matrix in, matrix out ){
	const int l = L;

	assert( !( N % l ) );

	for( int row = 0; row < N; row += l ){
		for( int col = 0; col < N; col += l ){
			
			// do transpose on this submatrix
			const int rlimit = row + l;
			const int climit = col + l;

			for( int i = row; i < rlimit; i++ ){
				for( int j = col; j < climit; j++ ){
					out[j][i] = in[i][j];	
				}
			}
		}
	}

}

#define OBLIV_MIN	4	// number of words in submatrix when cache oblivious gives up	

void recur_coblivous( matrix in, matrix out, int row_off, int col_off, int m, int n ){
	if( m > OBLIV_MIN || n > OBLIV_MIN ){
		if( n >= m ){
			const int half_n = n / 2;
			recur_coblivous( in, out, row_off, col_off, m, half_n );
			recur_coblivous( in, out, row_off, col_off + half_n, m, half_n ); 
		} else {
			const int half_m = m / 2;
			recur_coblivous( in, out, row_off, col_off, half_m, n );
			recur_coblivous( in, out, row_off + half_m, col_off, half_m, n ); 
		}

	} else {
		const int rlimit = row_off + n;
		const int climit = col_off + m;
		for( int i = row_off; i < rlimit; i++ ){
			for( int j = col_off; j < climit; j++ ){
				out[j][i] = in[i][j];
			}
		}
	}
}

void coblivous( matrix in, matrix out ){
	return recur_coblivous( in, out, 0, 0, N, N ); 
}

typedef void (*transpose)(matrix, matrix); 


uint32_t trial( transpose f, int iter, matrix in, matrix out ){
	struct timeval tv;
	gettimeofday( &tv, NULL );
	uint32_t start = tv.tv_sec * 1000 + tv.tv_usec / 1000;

	for( int i = 0; i < iter; i++ )
		f( in, out );


	gettimeofday( &tv, NULL );
	uint32_t end = tv.tv_sec * 1000 + tv.tv_usec / 1000;

	return end - start;
}


uint32_t experiment( char* name, transpose f, int trials, int iter, matrix in, matrix out ){
	uint32_t result[ trials ];

	for( int j = 0; j < trials; j++ ){
		result[ j ] = trial( f, iter, in, out );
	}

	printf("%s: ", name );
	for( int j = 0; j < trials; j++ )
		printf("%d ", result[j] );

	printf("\n");
}

// remember its out of situ so output of transpose is always the same no matter how many iterations
int verify( matrix m ){
	for( int i = 0; i < N; i++ ){
		for( int j = 0; j < N; j++ ){
			if( m[i][j] != ( j * N ) + ( i + 1 ) )
				assert( 0 );
		}
	}	
}

/* store in bss not stack to stop segfault */
matrix m, out;

int main( int argc, char* argv[] ){
	init( m );

#if 1 
	experiment( "naive", naive, 3, 100, m, out );
	experiment( "cache aware", caware, 3, 100, m, out );
	experiment( "cache oblivous", coblivous, 3, 100, m, out );

	// use to verify out individual of each
//	verify( out );	
#else		// unit testing 
	print( m );
	coblivous( m, out );
	print( out );
#endif
	return 0;
}


