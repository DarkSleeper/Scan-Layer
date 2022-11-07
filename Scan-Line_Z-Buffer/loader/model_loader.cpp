#include "model_loader.h"
#include <sstream>


ImportedModel::ImportedModel()
{

}

ImportedModel::ImportedModel(const char* filePath)
{
	ModelImporter modelImporter = ModelImporter();
	modelImporter.parseOBJ(filePath);

	_numVertices = modelImporter.getNumVertices();
	_triangle_indexes = modelImporter.getTriangleVertices();

	vector<float> origin_verts = modelImporter.getOriginVertices();
	vector<float> tcs = modelImporter.getTextureCoordinates();
	vector<float> normals = modelImporter.getNormals();

	for (int i = 0; i < _numVertices; i++)
	{
		_origin_vertices.push_back(glm::vec3(origin_verts[i * 3 + 0], origin_verts[i * 3 + 1], origin_verts[i * 3 + 2]));
		_texCoords.push_back(glm::vec2(tcs[i * 2 + 0], tcs[i * 2 + 1]));
		_normalVecs.push_back(glm::vec3(normals[i * 3 + 0], normals[i * 3 + 1], normals[i * 3 + 2]));
	}
}

int ImportedModel::getNumVertices()
{
	return std::move(_numVertices);
}

std::vector<int> ImportedModel::getTriangleIndexes()
{
	return std::move(_triangle_indexes);
}

std::vector<glm::vec2> ImportedModel::getTextureCoords()
{
	return std::move(_texCoords);
}

std::vector<glm::vec3> ImportedModel::getNormals()
{
	return std::move(_normalVecs);
}

std::vector<glm::vec3> ImportedModel::getOriginVertices()
{
	return std::move(_origin_vertices);
}

//

ModelImporter::ModelImporter()
{

}

void ModelImporter::parseOBJ(const char* filePath)
{
	float x = 0.f, y = 0.f, z = 0.f;
	string content;
	ifstream fileStream(filePath, ios::in);
	string line = "";

	bool init_face = true;
	while (!fileStream.eof())
	{
		getline(fileStream, line);
		if (line.compare(0, 2, "v ") == 0)
		{
			std::stringstream ss(line.erase(0, 1));
			ss >> x >> y >> z;
			//ss >> x; ss >> y; ss >> z;
			_vertVals.push_back(x);
			_vertVals.push_back(y);
			_vertVals.push_back(z);
		}
		if (line.compare(0, 2, "vt") == 0)
		{
			std::stringstream ss(line.erase(0, 2));
			ss >> x >> y;
			_stVals.push_back(x);
			_stVals.push_back(y);
		}
		if (line.compare(0, 2, "vn") == 0)
		{
			std::stringstream ss(line.erase(0, 2));
			ss >> x >> y >> z;
			_normVals.push_back(x);
			_normVals.push_back(y);
			_normVals.push_back(z);
		}
		if (line.compare(0, 1, "f") == 0)
		{
			if (init_face) {
				_textureCoords.resize(_vertVals.size() / 3 * 2);
				_normals.resize(_vertVals.size() / 3 * 3);
				init_face = false;
			}
			string oneCorner, v, t, n;
			std::stringstream ss(line.erase(0, 2));
			int idx[3];
			for (int i = 0; i < 3; i++)
			{
				getline(ss, oneCorner, ' ');
				//getline(ss, oneCorner, " ");
				stringstream oneCornerSS(oneCorner);
				getline(oneCornerSS, v, '/');
				getline(oneCornerSS, t, '/');
				getline(oneCornerSS, n, '/');

				idx[i] = stoi(v) - 1;

				int vertRef = idx[i] * 3;   
				int tcRef = (stoi(t) - 1) * 2;
				int normRef = (stoi(n) - 1) * 3;

				_triangleVerts.push_back(idx[i]);

				_textureCoords[idx[i] * 2 + 0] = _stVals[tcRef];
				_textureCoords[idx[i] * 2 + 1] = _stVals[tcRef + 1];

				_normals[idx[i] * 3 + 0] = _normVals[normRef];
				_normals[idx[i] * 3 + 1] = _normVals[normRef + 1];
				_normals[idx[i] * 3 + 2] = _normVals[normRef + 2];
			}
		}
	}
}

int ModelImporter::getNumVertices()
{
	return (_vertVals.size() / 3);
}

std::vector<int> ModelImporter::getTriangleVertices()
{
	return std::move(_triangleVerts);
}

std::vector<float> ModelImporter::getTextureCoordinates()
{
	return std::move(_textureCoords);
}

std::vector<float> ModelImporter::getNormals()
{
	return std::move(_normals);
}

std::vector<float> ModelImporter::getOriginVertices() 
{
	return std::move(_vertVals);
}
