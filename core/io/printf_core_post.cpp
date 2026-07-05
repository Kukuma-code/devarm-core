	while( (get_next(ch)) != 0 ){
		if ( ch == 0x5c ) { // 0x5c is escape.
			get_next(ch); table[cnt++]=(char)ch; continue; }
		if ( ch != '%' ) { table[cnt++]=(char)ch; continue; }

		get_next(ch); // % format bellow
		width=status=0; small_digit=6; pad=' ';
		if ( subtable[0] != 0 ) 
			for ( i=0; i< SUBTABLE_DIGIT ; i++ ) { subtable[i]=0; }
		if ( ch == '-' ) { status|=ISRIGHT; get_next(ch); }
		if ( ch == '0' ) { pad = '0' ; get_next(ch); }
		while ( _DIGIT_ ) {
				width=width*radix+(ch-'0');
				if ( width > REAL_DIGIT ) width=REAL_DIGIT;
				get_next(ch);
		}
		if ( ch == '.' ) { status|=HAVE_SMALL; get_next(ch);
			small_digit=0;
			while ( _DIGIT_ ) { 
				small_digit=small_digit*radix+(ch-'0');
				if ( small_digit > SUBTABLE_DIGIT ) small_digit=SUBTABLE_DIGIT;
				get_next(ch);	}
		}
		if ( ch == 'l' ) { status|=ISLONG; get_next(ch); }
		if ( !ch ) break;
		i=0;
		switch(ch){
		case 's':
			tmp= va_arg(argp,char*); while ( *tmp != 0 ) table[cnt++]=*tmp++; continue;
		case 'c':
			table[cnt++]=(char)va_arg(argp,int); continue;
		case 'u' : 
			if ( status & ISLONG){
				val2=va_arg(argp,unsigned long long);
			if (val2== 0) { subtable[i++]='0';break; }
			} else {
				val2=va_arg(argp,unsigned long);
			if (val2== 0) { subtable[i++]='0';break; }
			}
			do {
				n=val2%10; n+=0x30; subtable[i++]=(char)n; val2=val2/10;
			} while (val2 != 0);
			break;
		case 'd' : case 'i' : case 'n' : 
			if ( status & ISLONG){
				val=va_arg(argp,long long);
// // 				if ( !(ch=lntoad(val,&table[cnt],width,pad))) cnt+=width; else cnt+=ch;
				if (val== 0) { subtable[i++]='0';;break; }
				if ( val >> 0x3f ) { // lowest value can't reverse normally.
					status|=ISSIGN; val+=1; val=-val; n=val%10; n+=0x31;
					if ( n >= 0x3a ) { n-=0x0a;status|=ISCARRY;}
					subtable[i++]=(char)n; val=val/10;}
				if (val<0) { status|=ISSIGN; val=-val; }

			} else {
// //  				get_value(int);
// // 				if ( !(ch=ntoad(val,&table[cnt],width,pad))) cnt+=width; else cnt+=ch;
// // 			}
				get_value(int);
				if (val== 0) { subtable[i++]='0';break; }
				if (val==(int)0x80000000) { // lowest value can't reverse normally.
					status|=ISSIGN; val+=1; val=-val; n=val%10; n+=0x31;
					subtable[i++]=(char)n; val=val/10;}
				if (val<0) { status|=ISSIGN; val=-val; }
			}
			if ( val != 0 ) {
				do {
					n=val%10; n+=0x30; subtable[i++]=(char)n; val=val/10;
				} while (val != 0);
				if ( status & ISCARRY) subtable[i-1]+=1;
			}
			break;
		case 'p':
		case 'X' :
			table[cnt++]='0';table[cnt++]='x';
		case 'x' :
			if (status & ISLONG ) {
				val2=va_arg(argp,unsigned long long);
				lntoah(val2,&table[cnt],width);
				if (width == 0) cnt+=0x10; else cnt+=width;

// 				if ( (ch=( width *4 )) > DOUBLE_DIGIT || width == 0) ch =DOUBLE_DIGIT;
// 				for (j=ch-4; j>=0 ; j=j-4){
// 					n = ( val2 >> j) & 0x0f;
// 					if ( n >= 0x0a ) n += 0x27;
// 					n += 0x30;
// 					table[cnt++]=n;
// 				}
			}
			else{
				get_value(int);
 				ntoah(val,&table[cnt],width);
 				if (width == 0) cnt+=0x08; else cnt+=width;

// 				if ( (ch=( width *4 )) > LONG_DIGIT || width == 0) ch =LONG_DIGIT;
// 				for (j=ch-4; j>=0 ; j=j-4){
// 					n = ( val >> j) & 0x0f;
// 					if ( n >= 0x0a ) n += 0x27;
// 					n += 0x30;
// 					table[cnt++]=n;
// 				}
			}
			break;
		case 'B':
			table[cnt++]='0';table[cnt++]='b';
		case 'b' :
			if ( status & ISLONG ){
				get_value(long long);
				j=DOUBLE_DIGIT-1;
			} else {
				get_value(int);
				j=LONG_DIGIT-1;
			}
			for(; j>=0 ; j--){
				n = ( val >> j ) & 0x01;
				n += 0x30;
				table[cnt++]=n;
			} 
			break;
		case 'V' :
			table[cnt++]='0';table[cnt++]='b';
		case 'v' :
			if ( status & ISLONG ){
				get_value(long long);
				j=DOUBLE_DIGIT-1;
			}else{
				get_value(long);
				j=LONG_DIGIT-1;
			}
			for(; j>=0 ; j--){
				n = ( val >> j ) & 0x01;
				n += 0x30;
				table[cnt++]=n;
				if ( (j % 4) == 0 ) table[cnt++]=' ';
			} 
			break;
		case 'f':
		case 'w':
			status|=ISFLOAT;
			vald=va_arg(argp, double);
			dtoa(vald, subtable);
			break;
		case 'o':
			val2=va_arg(argp,unsigned long);
			if (val2== 0) { subtable[i++]='0';break; }
			do {
				n=val2%8; n+=0x30; subtable[i++]=(char)n; val2=val2/8;
			} while (val2 != 0);
			break;
		default:
			va_end(argp); return NUL_(PRINTF_NO_MATCH);
		}
		// post process
#define pad_width val
		if ( status & ISFLOAT ){
			while ( subtable[i] != 0 && subtable[i] != '.' ) table[cnt++]=subtable[i++];
			table[cnt++]=subtable[i++];
			//			DBGDN(small_digit);
			val=0;
			if ( subtable[i+small_digit]+5 > (0x30+10) ){
				DBGDN(subtable[i+small_digit]);
				val=1;
				for ( j=small_digit; j>=0; j--){
					subtable[i+j]+=val;
					//					DBGDN2(j,subtable[i+j]);
					if ( subtable[i+j] >= ( 0x30+10) ) {subtable[i+j]-=0x30; val=1;}
					else break;
				}
			}
			while ( small_digit-- > 0 ) table[cnt++]=subtable[i++];
		} else {
			if ( subtable[0] ){
				//				i--;
// 				if ( width == 0 ) {
// 					if ( status & ISSIGN ) table[cnt++]='-';
// 					while ( i >= 0 ) { table[cnt++]=subtable[i--]; }
// 				}
				if ( width == 0 ) {
					if (status & ISSIGN)subtable[i]='-'; else --i;
					while ( i >= 0 ) { table[cnt++]=subtable[i--]; }
				}
				else {
					if ( width > SUBTABLE_DIGIT ) err=NUL_(PRINTF_WIDTH_OVER);
					if ( pad == '0' ) { if (status & ISSIGN) {table[cnt++]='-';--width;--i;}else --i;}
					else {if (status & ISSIGN) subtable[i]='-'; else --i;}
					if ( width <= i ) err=NUL_(NTOAD_WIDTH_OVER);
					pad_width=width-i-1;
					if ( !(status & ISRIGHT) ) for ( ; pad_width>0; --pad_width ) table[cnt++]=pad;
					for ( ; i>=0&&width > 0 ; --i , --width ) table[cnt++]=subtable[i];
					if ( (status & ISRIGHT) ) for ( ; pad_width>0; --pad_width) table[cnt++]=pad;
				}
// 				else { pad_width=width-i-1;
// 					if ( pad_width > SUBTABLE_DIGIT ) err = NUL_(FPRINTF_CUTDIGIT);
// 					if ( pad_width && !(status & ISRIGHT) ) for ( j=pad_width; j>0; j--) table[cnt++]=pad;
// 					if ( status & ISSIGN ) table[cnt++]='-';
// 					for ( ; i>=0&&width > 0 ; i-- , width -- ) table[cnt++]=subtable[i];
// 					if ( pad_width && (status & ISRIGHT) ) for ( j=pad_width; j>0; j--) table[cnt++]=pad;
// 				}
			}
		}
	}
	va_end(argp);
#undef get_next
#undef get_value
#undef ISSIGN
#undef ISRIGHT
#undef ISLONG
#undef ISFLOAT
#undef pad_width
