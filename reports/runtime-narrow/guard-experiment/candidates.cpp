using I = __int128;
using U = unsigned __int128;
using W = unsigned long long;
constexpr I max = I((~U{0}) >> 1);
constexpr I min = -max - 1;
extern "C" bool signed_builtin(I a, I b, I* out) {
    return !__builtin_mul_overflow(a, b, out);
}
extern "C" bool unsigned_builtin(I a, I b, I* out) {
    bool neg = (a < 0) != (b < 0);
    U ma = a < 0 ? U{0} - U(a) : U(a);
    U mb = b < 0 ? U{0} - U(b) : U(b);
    U product;
    if (__builtin_mul_overflow(ma, mb, &product) || product > (U(max) + U(neg))) return false;
    *out = I(neg ? U{0} - product : product);
    return true;
}
extern "C" bool split_unsigned(I a, I b, I* out) {
    bool neg = (a < 0) != (b < 0);
    U ma = a < 0 ? U{0} - U(a) : U(a);
    U mb = b < 0 ? U{0} - U(b) : U(b);
    U product;
    if (((ma | mb) >> 64) == 0)
        product = U(W(ma)) * W(mb);
    else if ((ma >> 64) != 0 && (mb >> 64) != 0)
        return false;
    else if (__builtin_mul_overflow(ma, mb, &product))
        return false;
    if (product > (U(max) + U(neg))) return false;
    *out = I(neg ? U{0} - product : product);
    return true;
}
extern "C" bool scale_builtin(I value, I* out) {
    return !__builtin_mul_overflow(value, I{1000000000000ULL}, out);
}
extern "C" bool scale_bound(I value, I* out) {
    constexpr I factor = I{1000000000000ULL};
    if (value < min / factor || value > max / factor) return false;
    *out = value * factor;
    return true;
}
extern "C" bool narrowed_operand(I a, I b, I* out) {
    bool neg = (a < 0) != (b < 0);
    U ma = a < 0 ? U{0} - U(a) : U(a);
    U mb = b < 0 ? U{0} - U(b) : U(b);
    U product;
    if ((ma >> 64) == 0) {
        if (__builtin_mul_overflow(mb, W(ma), &product)) return false;
    } else {
        if ((mb >> 64) != 0) return false;
        if (__builtin_mul_overflow(ma, W(mb), &product)) return false;
    }
    if (product > (U(max) + U(neg))) return false;
    *out = I(neg ? U{0} - product : product);
    return true;
}
extern "C" bool split_builtin(I a, I b, I* out) {
    bool neg = (a < 0) != (b < 0);
    U ma = a < 0 ? U{0} - U(a) : U(a);
    U mb = b < 0 ? U{0} - U(b) : U(b);
    U product;
    if (((ma | mb) >> 64) == 0) {
        (void)__builtin_mul_overflow(W(ma), W(mb), &product);
    } else if ((ma >> 64) != 0 && (mb >> 64) != 0)
        return false;
    else if (__builtin_mul_overflow(ma, mb, &product))
        return false;
    if (product > (U(max) + U(neg))) return false;
    *out = I(neg ? U{0} - product : product);
    return true;
}
extern "C" bool reject_large(I a, I b, I* out) {
    bool neg = (a < 0) != (b < 0);
    U ma = a < 0 ? U{0} - U(a) : U(a), mb = b < 0 ? U{0} - U(b) : U(b);
    if ((ma >> 64) != 0 && (mb >> 64) != 0) return false;
    U product;
    if (__builtin_mul_overflow(ma, mb, &product) || product > U(max) + U(neg)) return false;
    *out = I(neg ? U{0} - product : product);
    return true;
}
extern "C" bool separate_small(I a, I b, I* out) {
    bool neg = (a < 0) != (b < 0);
    U ma = a < 0 ? U{0} - U(a) : U(a), mb = b < 0 ? U{0} - U(b) : U(b);
    if (((ma | mb) >> 64) == 0) {
        U product = U(W(ma)) * W(mb);
        if (product > U(max) + U(neg)) return false;
        *out = I(neg ? U{0} - product : product);
        return true;
    }
    if ((ma >> 64) != 0 && (mb >> 64) != 0) return false;
    U product;
    if (__builtin_mul_overflow(ma, mb, &product) || product > U(max) + U(neg)) return false;
    *out = I(neg ? U{0} - product : product);
    return true;
}
extern "C" bool partial_product(I a, I b, I* out) {
    bool neg = (a < 0) != (b < 0);
    U ma = a < 0 ? U{0} - U(a) : U(a), mb = b < 0 ? U{0} - U(b) : U(b);
    auto ah = W(ma >> 64), bh = W(mb >> 64), al = W(ma), bl = W(mb);
    if (ah != 0 && bh != 0) return false;
    U product = U(al) * bl;
    if ((ah | bh) != 0) {
        U upper = (ah != 0 ? U(ah) * bl : U(bh) * al) + (product >> 64);
        if ((upper >> 64) != 0) return false;
        product = (upper << 64) | W(product);
    }
    if (product > U(max) + U(neg)) return false;
    *out = I(neg ? U{0} - product : product);
    return true;
}
