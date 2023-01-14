
 /* rsa.c
 *  Created on: Jan 3, 2023
 *  Author: kadir.yamac */
#include <rsa.h>

extern int payload[];
extern int payintt;

void encrypt(int input){
	int i = PublicKey;
	int res = 1;     // Initialize result

	input = input % N; // Update x if it is more than or equal to p

	while (i > 0)
	{
		// If y is odd, multiply x with result
		if (i & 1)
			res = (res*input) % N;

		// y must be even now
		i = i>>1; // y = y/2
		input = (input*input) % N;
	}
	payload[payintt] = res;

}

void decrypt(int input){
	int i = PrivateKey;
	int res = 1;     // Initialize result

	input = input % N; // Update x if it is more than or equal to p

	while (i > 0)
	{
		// If y is odd, multiply x with result
		if (i & 1)
			res = (res*input) % N;

		// y must be even now
		i = i>>1; // y = y/2
		input = (input*input) % N;
	}



}
