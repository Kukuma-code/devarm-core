#define ISSIGN 0x01
#define ISRIGHT 0x02
#define ISLONG 0x04
#define ISFLOAT 0x08
#define HAVE_SMALL 0x10
#define ISCARRY 0x20
#define get_next(x) x =*format++
#define get_value(x) val=va_arg(argp,x)
#define REAL_DIGIT 0x34
#define SUBTABLE_DIGIT DOUBLE_DIGIT
#define TABLE_DIGIT 0x200
	va_list argp;
	long ch, width, small_digit, status, pad=' ', cnt, i ,j;
	long long val;
	unsigned long long val2;
	long radix=10;
	double vald;
	unsigned long n, err=0;
	char subtable[SUBTABLE_DIGIT];
	char table[TABLE_DIGIT];
	char* tmp;
	for ( i=0; i< TABLE_DIGIT ; i++ ) { table[i]=0; }
	for ( i=0; i< SUBTABLE_DIGIT ; i++ ) { subtable[i]=0; }
	va_start(argp, format);
	cnt=0;
