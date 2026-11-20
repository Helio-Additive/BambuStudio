#include "libslic3r/GCode/HelioMetadata.hpp"
#include <iostream>
#include <stdexcept>

namespace {
void require(bool condition) {
    if (!condition) throw std::runtime_error("Helio metadata regression");
}
}

int main()
{
    using namespace Slic3r::HelioMetadata;
    std::array<float, 9> values;
    values.fill(NAN);
    require(parse_warpage_fields("(wdm=0.0231,wdx=-0.0012,wdy=0.0046,wdz=0.0227,wr=0.422,wtg=-12.3457,wts=0.009449,whs=0.0187,wls=0.0003,element.index=42)", values));
    const std::array<float, 9> expected = {0.0231f,-0.0012f,0.0046f,0.0227f,0.422f,-12.3457f,0.009449f,0.0187f,0.0003f};
    for (size_t i = 0; i < values.size(); ++i)
        require(std::abs(values[i] - expected[i]) < 1e-6f);

    // Following partial annotations preserve existing inline values.
    require(parse_warpage_fields("(wr=0.7,wdx=nan,unknown=1)", values));
    require(values[4] == 0.7f && values[1] == expected[1]);
    require(!parse_warpage_fields("ti.mean=0.4,element.index=42", values));
    for (const auto invalid : {"", "abc", "0.5xyz", "nan", "inf", "-inf", "1e999"}) {
        for (const auto key : {"wdm", "wdx", "wdy", "wdz", "wr", "wtg", "wts", "whs", "wls"}) {
            const std::string annotation = std::string(key) + "=" + invalid;
            require(!parse_warpage_fields(annotation, values));
            float percentile = 0.25f;
            require(!parse_percentile(invalid, percentile));
            require(percentile == 0.25f);
        }
    }
    require(values[4] == 0.7f && values[7] == expected[7]);
    float percentile = NAN;
    require(parse_percentile("5e-1", percentile) && percentile == 0.5f);
    require(parse_percentile("0", percentile) && percentile == 0.f);
    values.fill(NAN);
    require(parse_warpage_fields("whs=0.2110", values));
    require(std::isnan(values[0]) && values[7] == 0.2110f && std::isnan(values[8]));
    std::cout << "Helio warpage metadata regressions passed\n";
}
