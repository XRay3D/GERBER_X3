///********************************************************************************
// * Author    :  Damir Bakiev                                                    *
// * Version   :  na                                                              *
// * Date      :  ХХ ХХХ 2025                                                 *
// * Website   :  na                                                              *
// * Copyright :  Damir Bakiev 2016-2025                                          *
// * License   :                                                                  *
// * Use, modification & distribution is subject to Boost Software License Ver 1. *
// * http://www.boost.org/LICENSE_1_0.txt                                         *
// ********************************************************************************/
// #include "gc_utils.h"
// #include "app.h"
// #include "math.h"
// #include "project.h"
// #include <QFileInfo>

// namespace GCode {

// File::File(Params&& gcp)
//     : gcp_(std::move(gcp)) {
// }

// QString File::getLastDir() {
//     if (App::gcSettings().sameFolder() && !redirected)
//         lastDir = QFileInfo(App::project().name()).absolutePath();
//     else if (lastDir.isEmpty()) {
//         QSettings settings;
//         lastDir = settings.value("LastGCodeDir").toString();
//         if (lastDir.isEmpty())
//             lastDir = QFileInfo(App::project().name()).absolutePath();
//         settings.setValue("LastGCodeDir", lastDir);
//     }
//     return lastDir += '/';
// }

// void File::setLastDir(QString dirPath) {
//     dirPath = QFileInfo(dirPath).absolutePath();
//     if (App::gcSettings().sameFolder() && !redirected) {
//         redirected = QFileInfo(App::project().name()).absolutePath() != dirPath;
//         if (!redirected)
//             return;
//     }
//     if (lastDir != dirPath) {
//         lastDir = dirPath;
//         QSettings settings;
//         settings.setValue("LastGCodeDir", lastDir);
//     }
// }

// mvector<double> File::getDepths() {
//     const auto gDepth {gcp_.getDepth()};
//     if (gDepth < gcp_.getTool().passDepth() || qFuzzyCompare(gDepth, gcp_.getTool().passDepth()))
//         return {-gDepth - gcp_.getTool().getDepth()};

//    const int count = static_cast<int>(ceil(gDepth / gcp_.getTool().passDepth()));
//    const double depth = gDepth / count;
//    mvector<double> depths(count);
//    for (int i = 0; i < count; ++i)
//        depths[i] = (i + 1) * -depth;
//    depths.back() = -gDepth - gcp_.getTool().depth();
//    return depths;
//}

//} // namespace GCode
