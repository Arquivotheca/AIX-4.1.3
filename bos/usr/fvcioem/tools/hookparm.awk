/TRC_/ {
   match($0,/TRC_(OTHER|RECV|XMIT)\(/); i=RSTART+RLENGTH; h=RSTART; l=RLENGTH-1;
   match($0,/\)\;/);     j=RSTART;
   x = substr($0,i,j-i);
   split(x,a,",");
   printf("%s\n%s\n%s\n",a[2],a[3],a[4]);
}
