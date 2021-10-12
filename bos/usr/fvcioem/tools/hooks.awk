/TRC_/ {
   match($0,/TRC_(OTHER|RECV|XMIT)\(/); i=RSTART+RLENGTH; h=RSTART; l=RLENGTH-1;
   match($0,/\)\;/);     j=RSTART;
   x = substr($0,i,j-i);
   split(x,a,",");
   printf("%s\t%s\t%d\t%s\t%s\t%s\t%s\n",substr($0,h,l),FILENAME,FNR,a[1],a[2],a[3],a[4]);
}
