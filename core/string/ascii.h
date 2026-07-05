#ifndef CORE_STRING_ASCII_H
#define CORE_STRING_ASCII_H
/*********************************************
 * core/string: ASCII 文字列プリミティブ (移植可能・HW非依存)
 *
 * 旧 include/a.h の純粋な文字列操作のみを抽出し、密な 1 行記述を可読な
 * 形に整形したもの。ロジックは原典と同一。putchar 依存の出力系 (puts/put*)
 * は core/io/aout.h に分離した。
 *
 * 命名は現行の terse 名を維持 (rename は最終パスで一括実施):
 *   alen=strlen, acpy=strcpy, ancpy=strncpy, aset=memset(char),
 *   acmp=strcmp, ancmp=strncmp, acat=strcat, ancat=strncat
 * 引数順は原典の (src, dst) を維持 (標準 libc の (dst, src) とは逆)。
 *********************************************/

/* 文字列長 (strlen) */
inline unsigned int alen(const char* src){
	unsigned int ref;
	for (ref = 0; src[ref]; ++ref);
	return ref;
}
inline void alen(const char* src, unsigned int& ret){
	for (ret = 0; src[ret]; ++ret);
}

/* コピー (strcpy) : src -> dst */
inline void acpy(const void* _src, char* _dst){
	unsigned long i = 0;
	while (((char*)_src)[i] != 0){
		_dst[i] = ((char*)_src)[i];
		++i;
	}
	_dst[i] = 0;
}

/* 長さ制限コピー (strncpy) : src -> dst, 最大 cnt */
inline void ancpy(const void* src, char* _dst, unsigned int cnt){
	unsigned int i = 0;
	while ((i < cnt) && (_dst[i] = ((char*)src)[i++]));
	_dst[i] = 0;
}

/* 埋める (memset for char) : dst を val で cnt 個 */
inline void aset(char val, char* dst, unsigned int cnt){
	for (unsigned int i = 0; i < cnt; ++i)
		dst[i] = val;
}

/* 比較 (strcmp) */
inline int acmp(const char* src, const char* dst){
	int ret = 0;
	while (src[ret] != 0 && src[ret] == dst[ret]) ++ret;
	return src[ret] - dst[ret];
}
inline void acmp(const char* src, const char* dst, int& ret){
	ret = 0;
	while (src[ret] != 0 && src[ret] == dst[ret]) ++ret;
	ret = src[ret] - dst[ret];
}

/* 長さ制限比較 (strncmp) */
inline int ancmp(const char* src, const char* dst, unsigned int cnt){
	int ret = 0;
	while (1){
		if (ret >= (int)cnt){ --ret; break; }
		if (src[ret] == 0 || src[ret] != dst[ret]) break;
		++ret;
	}
	return src[ret] - dst[ret];
}
inline void ancmp(const char* src, const char* dst, unsigned int cnt, int& ret){
	ret = 0;
	while (1){
		if (ret >= (int)cnt){ --ret; break; }
		if (src[ret] == 0 || src[ret] != dst[ret]) break;
		++ret;
	}
	ret = src[ret] - dst[ret];
}

/* 連結 (strcat) : dst の末尾に src を追加 */
inline void acat(const char* src, char* dst){
	unsigned int i, srclen = alen(src), dstlen = alen(dst);
	for (i = 0; i < srclen; i++)
		dst[dstlen + i] = src[i];
	dst[dstlen + i] = '\0';
}

/* 長さ制限連結 (strncat) */
inline void ancat(const char* src, char* dst, unsigned int n){
	unsigned int i, srclen = alen(src), dstlen = alen(dst);
	for (i = 0; i < srclen && srclen < n; i++)
		dst[dstlen + i] = src[i];
	dst[dstlen + i] = '\0';
}

#endif /* CORE_STRING_ASCII_H */
