#include "geo/common.hpp"
#include "geo/core/types.hpp"
#include "geo/geos/functions/scalar.hpp"
#include "geo/geos/functions/common.hpp"
#include "geo/geos/geos_wrappers.hpp"

#include "duckdb/parser/parsed_data/create_scalar_function_info.hpp"
#include "duckdb/common/vector_operations/unary_executor.hpp"
#include "duckdb/common/vector_operations/binary_executor.hpp"


namespace geo {

namespace geos {

using namespace geo::core;

static void SimplifyPreserveTopologyFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &lstate = GEOSFunctionLocalState::ResetAndGet(state);
	BinaryExecutor::Execute<string_t, double, string_t>(args.data[0], args.data[1], result, args.size(), [&](string_t input, double distance) {
		auto geom = lstate.factory.Deserialize(input);
		auto geos_geom = lstate.ctx.FromGeometry(geom);
		auto simplified = geos_geom.SimplifyPreserveTopology(distance);
		auto simplified_geom = lstate.ctx.ToGeometry(lstate.factory, simplified.get());
		return lstate.factory.Serialize(result, simplified_geom);
	});
}

void GEOSScalarFunctions::RegisterStSimplifyPreserveTopology(ClientContext &context) {
	auto &catalog = Catalog::GetSystemCatalog(context);

	ScalarFunctionSet set("ST_SimplifyPreserveTopology");

	set.AddFunction(ScalarFunction({GeoTypes::GEOMETRY(), LogicalType::DOUBLE}, GeoTypes::GEOMETRY(), SimplifyPreserveTopologyFunction, nullptr, nullptr, nullptr, GEOSFunctionLocalState::Init));

	CreateScalarFunctionInfo info(std::move(set));
	info.on_conflict = OnCreateConflict::ALTER_ON_CONFLICT;
	catalog.CreateFunction(context, &info);
}

} // namespace geos

} // namespace geo