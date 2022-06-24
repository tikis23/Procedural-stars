#include "TerrainNoise.h"

/* discontinuous pseudorandom uniformly distributed in [-0.5, +0.5]^3 */
glm::dvec3 random3(glm::dvec3 c) {
	float j = 4096.0 * sin(glm::dot(c, glm::dvec3(17.0, 59.4, 15.0)));
	glm::dvec3 r;
	r.z = glm::fract(512.0 * j);
	j *= .125;
	r.x = glm::fract(512.0 * j);
	j *= .125;
	r.y = glm::fract(512.0 * j);
	return r * 2.0 - 1.0;
}

/* skew constants for 3d simplex functions */
const double F3 = 0.3333333;
const double G3 = 0.1666667;

/* 3d simplex noise */
double simplex3d(glm::dvec3 p) {
	/* 1. find current tetrahedron T and it's four vertices */
	/* s, s+i1, s+i2, s+1.0 - absolute skewed (integer) coordinates of T vertices */
	/* x, x1, x2, x3 - unskewed coordinates of p relative to each of T vertices*/

	/* calculate s and x */
	glm::dvec3 s = floor(p + glm::dot(p, glm::dvec3(F3)));
	glm::dvec3 x = p - s + glm::dot(s, glm::dvec3(G3));

	/* calculate i1 and i2 */
	glm::dvec3 e = glm::step(glm::dvec3(0.0), x - glm::dvec3(x.y, x.z, x.x));
	glm::dvec3 i1 = e * (1.0 - glm::dvec3(e.z, e.x, e.y));
	glm::dvec3 i2 = 1.0 - glm::dvec3(e.z, e.x, e.y) * (1.0 - e);

	/* x1, x2, x3 */
	glm::dvec3 x1 = x - i1 + glm::dvec3(G3);
	glm::dvec3 x2 = x - i2 + 2.0 * G3;
	glm::dvec3 x3 = x - 1.0 + 3.0 * G3;

	/* 2. find four surflets and store them in d */
	glm::dvec4 w, d;

	/* calculate surflet weights */
	w.x = dot(x, x);
	w.y = dot(x1, x1);
	w.z = dot(x2, x2);
	w.w = dot(x3, x3);

	/* w fades from 0.6 at the center of the surflet to 0.0 at the margin */
	w = glm::max(0.6 - w, 0.0);

	/* calculate surflet components */
	d.x = dot(random3(s), x);
	d.y = dot(random3(s + i1), x1);
	d.z = dot(random3(s + i2), x2);
	d.w = dot(random3(s + 1.0), x3);

	/* multiply d by w^4 */
	w *= w;
	w *= w;
	d *= w;

	/* 3. return the sum of the four surflets */
	return glm::dot(d, glm::dvec4(52.0));
}


double TerrainNoise::Multifractal(glm::dvec3 pos) {
	pos = pos * 10.0;
	double heightScale = 1;
	int    octaves = 5;
	double gain = 0;
	double lacunarity = 1;
	double offset = 0;
	double h = 0;

    double frequency = lacunarity, signal, weight;

    // Get the base signal (absolute value to create the ridges; square for sharper ridges)
    signal = offset - abs(simplex3d(pos));
    signal *= signal;
    float result = signal;

    float exponentArraySum = 1.0;
    for (int i = 1; i < octaves; i++)
    {
        // This could be precalculated
        float exponentValue = pow(frequency, -h);
        exponentArraySum += exponentValue;
        frequency *= lacunarity;

        pos *= lacunarity;
        weight = glm::clamp(signal * gain, 0.0, 1.0);

        // Get the next "octave" (only true octave if lacunarity = 2.0, right?)
        signal = offset - abs(simplex3d(pos));
        signal *= signal;
        signal *= weight;

        result += signal * exponentValue;
    }

    // Scale result to [0,1] (not true when offset != 1.0)
    result /= exponentArraySum;

    return result;
}
