	/* table[TABLE_DIGIT] の境界: リテラル/%s は 1 文字ずつ残量を確認し、変換は
	   開始前に最大出力長 (CONV_RESERVE) の空きを確認する。足りなければ以降を
	   切り捨てる (従来は無検査でスタック上の table を越えて書いていた)。
	   変換 1 個の最大出力: %lv = 0b + 64 桁 + 16 区切り = 82 < CONV_RESERVE。 */
#define CONV_RESERVE 0x60
	while( (get_next(ch)) != 0 ){
		if ( cnt >= TABLE_DIGIT - 1 ) break;
		if ( ch == 0x5c ) { // 0x5c is escape.
			get_next(ch); if ( !ch ) break;   /* 末尾の '\\' で終端を読み越さない */
			table[cnt++]=(char)ch; continue; }
		if ( ch != '%' ) { table[cnt++]=(char)ch; continue; }
		if ( cnt + CONV_RESERVE >= TABLE_DIGIT ) break;

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
		/* 'l' は原典の規約で 64bit (long long) を意味する (旧 test/conv は %ld に
		   long long を渡す)。標準の 'll' も同義として受理する。 */
		if ( ch == 'l' ) { status|=ISLONG; get_next(ch); if ( ch == 'l' ) get_next(ch); }
		if ( !ch ) break;
		i=0;
		switch(ch){
		case 's':
			tmp= va_arg(argp,char*);
			while ( *tmp != 0 && cnt < TABLE_DIGIT - 1 ) table[cnt++]=*tmp++;
			continue;
		case 'c':
			table[cnt++]=(char)va_arg(argp,int); continue;
		case 'u' : 
			if ( status & ISLONG){
				val2=va_arg(argp,unsigned long long);
			if (val2== 0) { subtable[i++]='0';break; }
			} else {
				/* 非 l の %u は unsigned int で受ける (LP64 で unsigned long として読むと
				   上位 32bit が不定値になる。ILP32 では同幅で挙動不変)。 */
				val2=va_arg(argp,unsigned int);
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
		case 'p': {
			/* ポインタは pointer 幅で受ける (int で読むと LP64 で下位 32bit に切れる)。
			   既定幅は sizeof(void*)*2 桁 = ILP32 では従来どおり 8 桁。 */
			table[cnt++]='0';table[cnt++]='x';
			long pw = width ? width : (long)(sizeof(void*) * 2);
			ulntoah((unsigned long long)(uintptr_t)va_arg(argp,void*),&table[cnt],(int)pw);
			cnt += (pw * 4 > DOUBLE_DIGIT) ? DOUBLE_DIGIT / 4 : pw;
			break; }
		case 'X' :
			table[cnt++]='0';table[cnt++]='x';
			[[fallthrough]];   /* 接頭辞 0x を付けて 'x' と同じ変換へ */
		case 'x' :
		{
			/* 桁数 = width (0 なら最大桁)。最大桁 (8 / l で 16) を超える width は
			   従来 cnt だけ進めて table に NUL を残し出力が切れていた。超過分は
			   pad 文字 (右詰め '-' 指定時は空白を後置) で埋める。 */
			long hd = ( status & ISLONG ) ? 0x10 : 0x08;
			long dg = ( width == 0 || width > hd ) ? hd : width;
			long extra = ( width > hd ) ? width - hd : 0;
			if ( !(status & ISRIGHT) ) for ( ; extra>0; --extra ) table[cnt++]=(char)pad;
			if (status & ISLONG ) {
				val2=va_arg(argp,unsigned long long);
				ulntoah(val2,&table[cnt],(int)dg);
				cnt+=dg;

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
 				ntoah((int)val,&table[cnt],(int)dg);
 				cnt+=dg;

// 				if ( (ch=( width *4 )) > LONG_DIGIT || width == 0) ch =LONG_DIGIT;
// 				for (j=ch-4; j>=0 ; j=j-4){
// 					n = ( val >> j) & 0x0f;
// 					if ( n >= 0x0a ) n += 0x27;
// 					n += 0x30;
// 					table[cnt++]=n;
// 				}
			}
			for ( ; extra>0; --extra ) table[cnt++]=' ';   /* 右詰め時の後置 */
			break;
		}
		case 'B':
			table[cnt++]='0';table[cnt++]='b';
			[[fallthrough]];   /* 接頭辞 0b を付けて 'b' と同じ変換へ */
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
			[[fallthrough]];   /* 接頭辞 0b を付けて 'v' と同じ変換へ */
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
			/* 精度 small_digit (既定 6, %.0f 可) で正しく丸めた文字列を得る。
			   精度上限は dtoa_prec が DTOA_PREC_MAX へ丸め、出力は subtable に収まる。 */
			dtoa_prec(vald, subtable, (int)small_digit);
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
			/* subtable は dtoa_prec が丸め済みの完成文字列。ここでは field width だけ
			   適用する (従来はここで再丸めし、切り捨て桁へ加算する誤りと '9' 繰上げで
			   制御文字を出す誤りがあった)。 */
			for ( j=0; subtable[j] != 0; ++j ) ;
			pad_width = ( width > j ) ? width - j : 0;
			i=0;
			if ( !(status & ISRIGHT) ){
				if ( pad == '0' && subtable[0] == '-' ) table[cnt++]=subtable[i++];
				for ( ; pad_width>0; --pad_width ) table[cnt++]=(char)pad;
			}
			while ( subtable[i] != 0 ) table[cnt++]=subtable[i++];
			for ( ; pad_width>0; --pad_width ) table[cnt++]=' ';
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
#undef CONV_RESERVE
