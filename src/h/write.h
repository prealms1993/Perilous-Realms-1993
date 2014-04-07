#define ID_TYPE unsigned short int

#define ID(id) { t=id; fwrite(&t,sizeof(ID_TYPE),1,fp); }
#define WriteF(v)   fwrite(&v,sizeof(v),1,fp);
#define Write(id,v)     if (v) { ID(id); fwrite(&(v),sizeof(v),1,fp); }

#define WriteS(id,v)    if (v) { ID(id); t=strlen(v)+1; ID(t); fwrite(v,t,1,fp); }

#define WriteA(id,v,nel,type) if (nel) { ID(id); temp=nel*sizeof(type);\
  fwrite(&temp,sizeof(temp),1,fp); fwrite(v,temp,1,fp); }

