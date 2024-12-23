#include "duckdb/parser/parsed_data/create_scalar_function_info.hpp"
#include "spatial/common.hpp"
#include "spatial/core/functions/scalar.hpp"
#include "spatial/core/functions/common.hpp"
#include "spatial/core/geometry/geometry.hpp"
#include "spatial/core/types.hpp"
#include "spatial/core/function_builder.hpp"

namespace spatial {

namespace core {

//------------------------------------------------------------------------------
// POINT_2D
//------------------------------------------------------------------------------
static void Point2DFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	D_ASSERT(args.data.size() == 2);
	auto count = args.size();

	auto &x = args.data[0];
	auto &y = args.data[1];

	x.Flatten(count);
	y.Flatten(count);

	auto &children = StructVector::GetEntries(result);
	auto &x_child = children[0];
	auto &y_child = children[1];

	x_child->Reference(x);
	y_child->Reference(y);

	if (count == 1) {
		result.SetVectorType(VectorType::CONSTANT_VECTOR);
	}
}

//------------------------------------------------------------------------------
// POINT_3D
//------------------------------------------------------------------------------
static void Point3DFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	D_ASSERT(args.data.size() == 3);
	auto count = args.size();

	auto &x = args.data[0];
	auto &y = args.data[1];
	auto &z = args.data[2];

	x.Flatten(count);
	y.Flatten(count);
	z.Flatten(count);

	auto &children = StructVector::GetEntries(result);
	auto &x_child = children[0];
	auto &y_child = children[1];
	auto &z_child = children[2];

	x_child->Reference(x);
	y_child->Reference(y);
	z_child->Reference(z);

	if (count == 1) {
		result.SetVectorType(VectorType::CONSTANT_VECTOR);
	}
}

//------------------------------------------------------------------------------
// POINT_4D
//------------------------------------------------------------------------------
static void Point4DFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	D_ASSERT(args.data.size() == 4);
	auto count = args.size();

	auto &x = args.data[0];
	auto &y = args.data[1];
	auto &z = args.data[2];
	auto &m = args.data[3];

	x.Flatten(count);
	y.Flatten(count);
	z.Flatten(count);
	m.Flatten(count);

	auto &children = StructVector::GetEntries(result);
	auto &x_child = children[0];
	auto &y_child = children[1];
	auto &z_child = children[2];
	auto &m_child = children[3];

	x_child->Reference(x);
	y_child->Reference(y);
	z_child->Reference(z);
	m_child->Reference(m);

	if (count == 1) {
		result.SetVectorType(VectorType::CONSTANT_VECTOR);
	}
}

//------------------------------------------------------------------------------
// GEOMETRY
//------------------------------------------------------------------------------
static void PointFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &lstate = GeometryFunctionLocalState::ResetAndGet(state);
	auto &arena = lstate.arena;
	auto &x = args.data[0];
	auto &y = args.data[1];
	auto count = args.size();

	BinaryExecutor::Execute<double, double, geometry_t>(x, y, result, count, [&](double x, double y) {
		return Geometry::Serialize(Point::CreateFromVertex(arena, VertexXY {x, y}), result);
	});
}

//------------------------------------------------------------------------------
// Register functions
//------------------------------------------------------------------------------
void CoreScalarFunctions::RegisterStPoint(DatabaseInstance &db) {

	FunctionBuilder::RegisterScalar(db, "ST_Point", [](ScalarFunctionBuilder &func) {
		func.AddVariant([](ScalarFunctionVariantBuilder &variant) {
			variant.AddParameter("x", LogicalType::DOUBLE);
			variant.AddParameter("y", LogicalType::DOUBLE);
			variant.SetReturnType(GeoTypes::GEOMETRY());
			variant.SetFunction(PointFunction);
			variant.SetInit(GeometryFunctionLocalState::Init);

			variant.SetDescription("Creates a GEOMETRY point");
		});

		func.SetTag("ext", "spatial");
		func.SetTag("category", "construction");
	});

	FunctionBuilder::RegisterScalar(db, "ST_Point2D", [](ScalarFunctionBuilder &func) {
		func.AddVariant([](ScalarFunctionVariantBuilder &variant) {
			variant.AddParameter("x", LogicalType::DOUBLE);
			variant.AddParameter("y", LogicalType::DOUBLE);
			variant.SetReturnType(GeoTypes::POINT_2D());
			variant.SetFunction(Point2DFunction);

			variant.SetDescription("Creates a POINT_2D");
		});

		func.SetTag("ext", "spatial");
		func.SetTag("category", "construction");
	});

	FunctionBuilder::RegisterScalar(db, "ST_Point3D", [](ScalarFunctionBuilder &func) {
		func.AddVariant([](ScalarFunctionVariantBuilder &variant) {
			variant.AddParameter("x", LogicalType::DOUBLE);
			variant.AddParameter("y", LogicalType::DOUBLE);
			variant.AddParameter("z", LogicalType::DOUBLE);
			variant.SetReturnType(GeoTypes::POINT_3D());
			variant.SetFunction(Point3DFunction);

			variant.SetDescription("Creates a POINT_3D");
		});

		func.SetTag("ext", "spatial");
		func.SetTag("category", "construction");
	});

	FunctionBuilder::RegisterScalar(db, "ST_Point4D", [](ScalarFunctionBuilder &func) {
		func.AddVariant([](ScalarFunctionVariantBuilder &variant) {
			variant.AddParameter("x", LogicalType::DOUBLE);
			variant.AddParameter("y", LogicalType::DOUBLE);
			variant.AddParameter("z", LogicalType::DOUBLE);
			variant.AddParameter("m", LogicalType::DOUBLE);
			variant.SetReturnType(GeoTypes::POINT_4D());
			variant.SetFunction(Point4DFunction);

			variant.SetDescription("Creates a POINT_4D");
		});

		func.SetTag("ext", "spatial");
		func.SetTag("category", "construction");
	});
}

} // namespace core

} // namespace spatial