/* The Computer Language Benchmarks Game
   https://salsa.debian.org/benchmarksgame-team/benchmarksgame/

   contributed by Mark C. Lewis
   modified slightly by Chad Whipkey
   converted from java to c++,added sse support, by Branimir Maksimovic
   modified by Vaclav Zeman
   modified by Vaclav Haisman to use explicit SSE2 intrinsics

   rust version contributed by Ilia Schelokov
   advance() converted to c++, minor cleanups by Ivan Sorokin
*/

#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>

struct vec3
{
    constexpr vec3()
        : x(0.)
        , y(0.)
        , z(0.)
    {}

    constexpr vec3(double x, double y, double z)
        : x(x)
        , y(y)
        , z(z)
    {}

    double x, y, z;
};

double sum_squares(vec3 v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

double magnitude(vec3 v, double dt)
{
    double sum = sum_squares(v);
    return dt / (sum * std::sqrt(sum));
}

vec3 operator+(vec3 a, vec3 b)
{
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

vec3 operator-(vec3 a, vec3 b)
{
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

vec3 operator*(vec3 a, double b)
{
    return {a.x * b, a.y * b, a.z * b};
}

vec3& operator+=(vec3& a, vec3 b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}

vec3& operator-=(vec3& a, vec3 b)
{
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    return a;
}

constexpr size_t BODIES_COUNT = 5;
constexpr size_t INTERACTIONS = BODIES_COUNT * (BODIES_COUNT - 1) / 2;

constexpr double PI = 3.141592653589793;
constexpr double SOLAR_MASS = 4 * PI * PI;
constexpr double DAYS_PER_YEAR = 365.24;

struct body
{
    vec3 position;
    vec3 velocity;
    double mass;

    body() = default;

    void offset_momentum(vec3 p)
    {
        velocity = p * (-1. / SOLAR_MASS);
    }
};

constexpr body jupiter =
{
    .position = {4.84143144246472090e+00, -1.16032004402742839e+00, -1.03622044471123109e-01},
    .velocity = {1.66007664274403694e-03 * DAYS_PER_YEAR, 7.69901118419740425e-03 * DAYS_PER_YEAR, -6.90460016972063023e-05 * DAYS_PER_YEAR},
    .mass = 9.54791938424326609e-04 * SOLAR_MASS,
};

constexpr body saturn =
{
    .position = {8.34336671824457987e+00, 4.12479856412430479e+00, -4.03523417114321381e-01},
    .velocity = {-2.76742510726862411e-03 * DAYS_PER_YEAR, 4.99852801234917238e-03 * DAYS_PER_YEAR, 2.30417297573763929e-05 * DAYS_PER_YEAR},
    .mass = 2.85885980666130812e-04 * SOLAR_MASS,
};

constexpr body uranus =
{
    .position = {1.28943695621391310e+01, -1.51111514016986312e+01, -2.23307578892655734e-01},
    .velocity = {2.96460137564761618e-03 * DAYS_PER_YEAR, 2.37847173959480950e-03 * DAYS_PER_YEAR, -2.96589568540237556e-05 * DAYS_PER_YEAR},
    .mass = 4.36624404335156298e-05 * SOLAR_MASS,
};

constexpr body neptune =
{
    .position = {1.53796971148509165e+01, -2.59193146099879641e+01, 1.79258772950371181e-01},
    .velocity = {2.68067772490389322e-03 * DAYS_PER_YEAR, 1.62824170038242295e-03 * DAYS_PER_YEAR, -9.51592254519715870e-05 * DAYS_PER_YEAR},
    .mass = 5.15138902046611451e-05 * SOLAR_MASS,
};

constexpr body sun =
{
    .mass = SOLAR_MASS,
};

class nbody_system {
private:
    std::array<body, 5> bodies;

public:
    constexpr nbody_system()
        : bodies{{sun, jupiter, saturn, uranus, neptune}}
    {
        vec3 p;
        for (size_t i = 0; i != bodies.size(); ++i)
            p += bodies[i].velocity * bodies[i].mass;

        bodies[0].offset_momentum(p);
    }

    void advance(double dt)
    {
        std::array<vec3, INTERACTIONS> d_positions;
        std::array<double, INTERACTIONS> magnitudes;

        size_t k = 0;
        for (size_t i = 0; i != bodies.size(); ++i)
            for (size_t j = i + 1; j != bodies.size(); ++j)
                d_positions[k++] = bodies[i].position - bodies[j].position;

        for (size_t i = 0; i != INTERACTIONS; ++i)
            magnitudes[i] = magnitude(d_positions[i], dt);

        k = 0;
        for (size_t i = 0; i != bodies.size(); ++i)
        {
            for (size_t j = i + 1; j != bodies.size(); ++j)
            {
                vec3 d_pos = d_positions[k];
                double mag = magnitudes[k];
                bodies[i].velocity -= d_pos * (bodies[j].mass * mag);
                bodies[j].velocity += d_pos * (bodies[i].mass * mag);
                ++k;
            }
        }

        for (size_t i = 0; i != bodies.size(); ++i)
            bodies[i].position += bodies[i].velocity * dt;
    }

    double energy() const
    {
        double e = 0.0;

        for (size_t i = 0; i != bodies.size(); ++i)
        {
            body const & ibody = bodies[i];
            double dx, dy, dz, distance;
            e += 0.5 * ibody.mass * sum_squares(ibody.velocity);

            for (size_t j = i + 1; j != bodies.size(); ++j)
            {
                body const & jbody = bodies[j];
                vec3 d = ibody.position - jbody.position;
                e -= (ibody.mass * jbody.mass) / sqrt(sum_squares(d));
            }
        }
        return e;
    }
};

int main(int argc, char* argv[])
{
    int n = atoi(argv[1]);

    nbody_system bodies;
    printf("%.9f\n", bodies.energy());
    for (int i=0; i<n; ++i)
        bodies.advance(0.01);
    printf("%.9f\n", bodies.energy());
}
