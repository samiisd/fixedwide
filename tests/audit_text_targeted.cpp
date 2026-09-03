#include <fixedwide/all.hpp>
#include <iostream>
#include <string_view>
using namespace fixedwide;
int main(){int f=0;auto chk=[&](bool ok,const char*n){std::cout<<(ok?"PASS ":"FAIL ")<<n<<'\n';if(!ok)++f;};
 const std::string_view max256="57896044618658097711785492504343953926634992332820282019728792003956564819967";
 const std::string_view min256="-57896044618658097711785492504343953926634992332820282019728792003956564819968";
 auto pmax=parse<Fixed256<0>>(max256);chk(pmax&&*pmax==Fixed256<0>::max(),"parse Fixed256 max");
 auto pmin=parse<Fixed256<0>>(min256);chk(pmin&&*pmin==Fixed256<0>::min(),"parse Fixed256 min");
 char b[256]; auto ten18=parse_u256("1000000000000000000");auto n=to_chars(b,sizeof b,*ten18);std::string_view got(b,n?*n:0);std::cout<<"wide 1e18 formatted="<<got<<'\n';chk(n&&got=="1000000000000000000","format u256 1e18");
 auto fv=Fixed256<18>::from_raw(wide::int256(1'000'000'000'000'000'000ULL));auto z=to_chars(b,sizeof b,fv);std::string_view fg(b,z?*z:0);std::cout<<"Fixed256 raw=1e18 formatted="<<fg<<'\n';chk(z&&fg=="1.000000000000000000","format Fixed256<18> one");
 std::cout<<"failures="<<f<<'\n';return f?1:0;}
