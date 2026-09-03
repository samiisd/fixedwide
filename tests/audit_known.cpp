#include <fixedwide/all.hpp>
#include <iostream>
#include <limits>
using namespace fixedwide;

template<class E> bool has_error(const E& e, ArithmeticError want){return !e && e.error()==want;}
int main(){
 int failures=0;
 auto check=[&](bool ok,const char* name){std::cout<<(ok?"PASS ":"FAIL ")<<name<<'\n';if(!ok)++failures;};
 auto a=from_integer<Fixed64<0>>(std::uint64_t{1});
 check(a && a->raw()==1,"from_integer Fixed64<0> uint64 1");
 auto b=from_integer<Fixed8<2>>(65536);
 check(has_error(b,ArithmeticError::overflow),"from_integer Fixed8<2> 65536 rejects overflow");
 auto c=from_integer<Fixed128<0>>(std::numeric_limits<std::uint64_t>::max());
 check(c && c->raw().high==0 && c->raw().low==~0ULL,"from_integer Fixed128<0> UINT64_MAX");
 auto cast128=fixed_cast<Fixed128<0>>(Fixed128<0>::min());
 check(cast128 && *cast128==Fixed128<0>::min(),"fixed_cast preserves Fixed128 minimum");
 auto cast256=fixed_cast<Fixed256<0>>(Fixed256<0>::min());
 check(cast256 && *cast256==Fixed256<0>::min(),"fixed_cast preserves Fixed256 minimum");
 auto unit=Fixed64<12>::from_raw(1);
 auto ce=mul_wide(unit,unit,Rounding::ceil);
 check(ce && ce->raw()==wide::int128(1),"mul_wide ceil positive");
 auto exact=mul_wide(Fixed64<12>::from_raw(1),Fixed64<12>::from_raw(1),Rounding::exact);
 check(has_error(exact,ArithmeticError::inexact),"mul_wide exact rejects inexact");
 auto neg=mul_wide(Fixed64<12>::from_raw(-1),Fixed64<12>::from_raw(1),Rounding::floor);
 check(neg && neg->raw()==wide::int128(-1),"mul_wide floor negative");
 auto nceil=mul_wide(Fixed64<12>::from_raw(-1),Fixed64<12>::from_raw(1),Rounding::ceil);
 check(nceil && nceil->raw()==wide::int128(0),"mul_wide ceil negative");
 std::cout<<"failures="<<failures<<'\n'; return failures?1:0;
}
