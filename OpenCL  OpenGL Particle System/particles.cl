typedef float4 point;
typedef float4 vector;
typedef float4 color;
typedef float4 sphere;


vector
Bounce( vector in, vector n )
{
	vector out = in - n*(vector)( 2.*dot(in.xyz, n.xyz) );
	out.w = 0.;
	return out;
}

vector
BounceSphere( point p, vector v, sphere s )
{
	vector n;
	n.xyz = fast_normalize( p.xyz - s.xyz );
	n.w = 0.;
	return Bounce( v, n );
}

bool
IsInsideSphere( point p, sphere s )
{
	float r = fast_length( p.xyz - s.xyz );
	return  ( r < s.w );
}

kernel
void
Particle( global point *dPobj, global vector *dVel, global color *dCobj )
{
	const float4 G       = (float4) ( 0., -9.8, 0., 0. );
	const float  DT      = 0.1;
	const sphere Sphere1 = (sphere)( -100., -800., 0.,  600. );
	const sphere Sphere2 = (sphere)( -900., -800., 0.,  600. );
	const sphere Sphere3 = (sphere)( 700., -800., 0.,  600. );
	int gid = get_global_id( 0 );

	// extract the position and velocity for this particle:
	point  p = dPobj[gid];
	vector v = dVel[gid];
	color  c = dCobj[gid];

	// remember that you also need to extract this particle's color
	// and change it in some way that is obviously correct


	// advance the particle:

	point  pp = p + v*DT + G*(point)( .5*DT*DT );
	vector vp = v + G*DT;
	pp.w = 1.;
	vp.w = 0.;

	// test against the first sphere here:

	if( IsInsideSphere( pp, Sphere1 ) )
		{
			vp = BounceSphere( p, v, Sphere1 );
			pp = p + vp*DT + G*(point)( .5*DT*DT );
			c = (float4)(.5f, .1f, 0., 0) + v*(float4)(.1f, .1f, .1f, .1f);
		}

	// test against the second sphere here:
	if( IsInsideSphere( pp, Sphere2 ) )
		{
			vp = BounceSphere( p, v, Sphere2 );
			pp = p + vp*DT + G*(point)( .5*DT*DT );
			c = (float4)(.4f, .0f, .3f, 0) + v*(float4)(.1f, .1f, .1f, .1f);
		}
		
	// test against the third sphere here:
	if( IsInsideSphere( pp, Sphere3 ) )
		{
			vp = BounceSphere( p, v, Sphere3 );
			pp = p + vp*DT + G*(point)( .5*DT*DT );
			c= (float4)(0.0f, 0.0f, 1.0f, 0) + v*(float4)(.1f, .1f, .1f, .1f);
		}

	dPobj[gid] = pp;
	dVel[gid]  = vp;
	dCobj[gid] = c;
}
