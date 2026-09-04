#include <fixedwide/all.hpp>
#include <limits>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
using namespace fixedwide;
int main(){int f=0;auto chk=[&](bool ok,const char*n){std::cout<<(ok?"PASS ":"FAIL ")<<n<<'\n';if(!ok)++f;};
 const double two63=std::ldexp(1.0,63);auto a=from_float<Fixed64<0>>(two63,Rounding::exact);if(a)std::cout<<"2^63 raw="<<a->raw()<<'\n';else std::cout<<"2^63 error="<<int(a.error())<<'\n';chk(!a&&a.error()==ArithmeticError::overflow,"Fixed64 rejects 2^63");
 const double near128=std::nextafter(std::ldexp(1.0,127),0.0);auto b=from_float<Fixed128<0>>(near128,Rounding::exact);std::cout<<"near int128 max result="<<(b?"value":"error")<<(b?0:int(b.error()))<<'\n';chk(bool(b),"Fixed128 accepts largest double below 2^127");
 auto c=from_float<Fixed256<0>>(1.1e76,Rounding::exact);std::cout<<"1.1e76 result="<<(c?"value":"error")<<(c?0:int(c.error()))<<'\n';chk(bool(c),"Fixed256 accepts representable 1.1e76");
 // long double is 80-bit x87 on x86-64, IEEE binary128 on some targets, and
 // plain double on arm64 macOS. The perturbation was hard-coded at 2^-60,
 // which needs 61 significand bits: on Apple silicon 1.0L+2^-60 IS 1.0L, so
 // the test asserted a precision the platform does not have and failed there.
 // Ask the platform what it has, and keep the perturbation big enough to still
 // move the 18th decimal, so this remains a real precision check everywhere.
 constexpr int ld_digits=std::numeric_limits<long double>::digits;
 constexpr int e=ld_digits-3<60?ld_digits-3:60;
 const long double ld=1.0L+std::ldexp(1.0L,-e);
 auto d=from_float<Fixed128<18>>(ld,Rounding::nearest_even);
 // (1 + 2^-e) * 10^18, rounded to nearest. 10^18 needs 42 significand bits, so
 // scaling it by a power of two is exact on every long double here.
 const wide::int128 expected(1'000'000'000'000'000'000LL+
     static_cast<long long>(std::llround(std::ldexp(1'000'000'000'000'000'000.0L,-e))));
 if(d)std::cout<<"long double digits="<<ld_digits<<" e="<<e<<" raw low="<<d->raw().low<<" high="<<d->raw().high<<'\n';
 chk(d&&d->raw()==expected,"long double conversion keeps the precision the platform's long double has");
 std::cout<<"failures="<<f<<'\n';return f?1:0;}
