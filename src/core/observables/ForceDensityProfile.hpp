#ifndef OBSERVABLES_FORCEDENSITYPROFILE_HPP
#define OBSERVABLES_FORCEDENSITYPROFILE_HPP

#include "ProfileObservable.hpp"

#include <vector>

namespace Observables {

class ForceDensityProfile : public ProfileObservable {
public:
  virtual int n_values() const override { return 3 * n_x_bins * n_y_bins * n_z_bins; }
  virtual std::vector<double> operator()(PartCfg &partCfg) const override {
    std::vector<size_t> n_bins{{static_cast<size_t>(n_x_bins),
                                static_cast<size_t>(n_y_bins),
                                static_cast<size_t>(n_z_bins)}};
    std::vector<std::pair<double, double>> limits{{std::make_pair(min_x, max_x),
                                                   std::make_pair(min_y, max_y),
                                                   std::make_pair(min_z, max_z)}};
    Utils::Histogram<double> histogram(n_bins, 3, limits);
    for (int id : ids) {
      auto const ppos = ::Vector<3, double>(folded_position(partCfg[id]));
      histogram.update(
          ppos, ::Vector<3, double>{{partCfg[id].f.f[0], partCfg[id].f.f[1],
                                     partCfg[id].f.f[2]}});
    }
    histogram.normalize();
    return histogram.get_histogram();
  }
};

} // Namespace Observables

#endif
