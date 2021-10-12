!/typedef/ && !/^struct/ && /struct / {
   p=match($0,/struct /);
   tmp=substr($0,RSTART+RLENGTH); 
   l=match(tmp,/[^a-zA-Z0-9_]/);
   s=substr(tmp,1,l-1);
   if(s!="")
   {
      if (a[s]==0) print FILENAME;
      a[s]=1;
   }
}

END {for(i in a) print i;}
