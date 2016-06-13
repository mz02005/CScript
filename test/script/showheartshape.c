/*这个测试打印两个心形的图形*/

function first()
{
	float y,x,z,f;
	for (y = 1.5f; y>-1.5f;y-=0.1f) {
		for(x=-1.5f;x<1.5f;x+=0.05f) {
			z=x*x+y*y-1;
			f=z*z*z-x*x*y*y*y;
			print(f <=0.0f ? ".:-=+*#%@".substr(f*-8.f,1) : " ");
		}
		print("\n");
	}
}

function f(x,y,z)
{
	float a = 
		x*x+9.0f/4*y*y+z*z-1;
	return
		a*a*a-x*x*z*z*z-9.f/80*y*y*z*z*z;
}

function h(x,z) {
	float y;
	for(y=1.f;y>=0.f;y-=0.001f)
		if(f(x,y,z) <=0.f)
			return y;
	return 0.f;
}

function sqrt(v)
{
	return pow(v,0.5f);
}

function main()
{
	float z,x,v,y0,ny,nx,nz,nd,d;
	for(z=1.5f;z>-1.5f;z-=0.05f) {
		for(x=-1.5f;x<1.5f;x+=0.025f) {
			v=f(x,0.f,z);
			if(v<=0.f) {
				y0 = h(x,z);
				ny=0.01f;
				nx=h(x+ny,z)-y0;
				nz=h(x,z+ny)-y0;
				nd=1.f/sqrt(nx*nx+ny*ny+nz*nz);
				d=(nx+ny-nz)*nd*0.5f+0.5f;
				print(".:-=+*#%@".substr(d*5.f,1));
			}
			else
				print(" ");
		}
		print("\n");
	}
}

println(time());
first();
main();
println(time());