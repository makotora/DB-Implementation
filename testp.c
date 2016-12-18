#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include "exhash.h"
#include "BF.h"
#include "EH_functions.h"

int main(void)
{
	BF_Init();
	//EH_CreateIndex("EH_test","name",'c',15*sizeof(char),3);
	EH_info * info = EH_OpenIndex("EH_test");
	EH_CloseIndex(info);
}