{
   if (substr($0,1,17) == "#define TIMESTAMP") 
   { 
      split($0,a); 
      split(a[5],b,".");
      b[1]++;
      "date +\".%y%m%d.7/27/94M%S\"" | getline d
      t = sprintf("Internal Build %4.4d%s",b[1],d);
      print "#define TIMESTAMP \"" t "\""
      print t | "cat 1>&2"
   }
   else
      print;
}
