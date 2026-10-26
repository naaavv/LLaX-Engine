#include "Renderer2D.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Buffer.h"
#include "Renderer/Shader.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <array>

namespace LLaX
{
    struct QuadVertex
    {
        glm::vec3 Position;
        glm::vec4 Color;
        glm::vec2 TexCoord;
        float TexIndex;
        float TilingFactor;

        // Editor-only / Mouse picking
        int EntityID;
    };

    struct LineVertex
    {
        glm::vec3 Position;
        glm::vec4 Color;
    };

    struct Renderer2DData
    {
        static const uint32_t MaxQuads = 20000;
        static const uint32_t MaxVertices = MaxQuads * 4;
        static const uint32_t MaxIndices = MaxQuads * 6;
        static const uint32_t MaxTextureSlots = 32;

        // Quad batch
        Ref<VertexArray> QuadVertexArray;
        Ref<VertexBuffer> QuadVertexBuffer;
        Ref<Shader> TextureShader;
        Ref<Texture2D> WhiteTexture;

        uint32_t QuadIndexCount = 0;
        QuadVertex* QuadVertexBufferBase = nullptr;
        QuadVertex* QuadVertexBufferPtr = nullptr;

        std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
        uint32_t TextureSlotIndex = 1; // 0 is 1x1 white texture

        glm::vec4 QuadVertexPositions[4];

        // Line batch
        static const uint32_t MaxLines = 20000;
        static const uint32_t MaxLineVertices = MaxLines * 2;

        Ref<VertexArray> LineVertexArray;
        Ref<VertexBuffer> LineVertexBuffer;
        Ref<Shader> LineShader;

        uint32_t LineVertexCount = 0;
        LineVertex* LineVertexBufferBase = nullptr;
        LineVertex* LineVertexBufferPtr = nullptr;

        float LineWidth = 2.0f;

        Renderer2D::Statistics Stats;
    };

    static Renderer2DData s_Data;

    static const char* s_TextureShaderSourceVertex = R"(
        #version 330 core
        layout(location = 0) in vec3 a_Position;
        layout(location = 1) in vec4 a_Color;
        layout(location = 2) in vec2 a_TexCoord;
        layout(location = 3) in float a_TexIndex;
        layout(location = 4) in float a_TilingFactor;
        layout(location = 5) in int a_EntityID;

        uniform mat4 u_ViewProjection;

        out vec4 v_Color;
        out vec2 v_TexCoord;
        out float v_TexIndex;
        out float v_TilingFactor;
        flat out int v_EntityID;

        void main()
        {
            v_Color = a_Color;
            v_TexCoord = a_TexCoord;
            v_TexIndex = a_TexIndex;
            v_TilingFactor = a_TilingFactor;
            v_EntityID = a_EntityID;
            gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
        }
    )";

    static const char* s_TextureShaderSourceFragment = R"(
        #version 330 core
        layout(location = 0) out vec4 o_Color;
        layout(location = 1) out int o_EntityID;

        in vec4 v_Color;
        in vec2 v_TexCoord;
        in float v_TexIndex;
        in float v_TilingFactor;
        flat in int v_EntityID;

        uniform sampler2D u_Textures[32];

        void main()
        {
            vec4 texColor = v_Color;
            switch (int(v_TexIndex))
            {
                case  0: texColor *= texture(u_Textures[ 0], v_TexCoord * v_TilingFactor); break;
                case  1: texColor *= texture(u_Textures[ 1], v_TexCoord * v_TilingFactor); break;
                case  2: texColor *= texture(u_Textures[ 2], v_TexCoord * v_TilingFactor); break;
                case  3: texColor *= texture(u_Textures[ 3], v_TexCoord * v_TilingFactor); break;
                case  4: texColor *= texture(u_Textures[ 4], v_TexCoord * v_TilingFactor); break;
                case  5: texColor *= texture(u_Textures[ 5], v_TexCoord * v_TilingFactor); break;
                case  6: texColor *= texture(u_Textures[ 6], v_TexCoord * v_TilingFactor); break;
                case  7: texColor *= texture(u_Textures[ 7], v_TexCoord * v_TilingFactor); break;
                case  8: texColor *= texture(u_Textures[ 8], v_TexCoord * v_TilingFactor); break;
                case  9: texColor *= texture(u_Textures[ 9], v_TexCoord * v_TilingFactor); break;
                case 10: texColor *= texture(u_Textures[10], v_TexCoord * v_TilingFactor); break;
                case 11: texColor *= texture(u_Textures[11], v_TexCoord * v_TilingFactor); break;
                case 12: texColor *= texture(u_Textures[12], v_TexCoord * v_TilingFactor); break;
                case 13: texColor *= texture(u_Textures[13], v_TexCoord * v_TilingFactor); break;
                case 14: texColor *= texture(u_Textures[14], v_TexCoord * v_TilingFactor); break;
                case 15: texColor *= texture(u_Textures[15], v_TexCoord * v_TilingFactor); break;
                case 16: texColor *= texture(u_Textures[16], v_TexCoord * v_TilingFactor); break;
                case 17: texColor *= texture(u_Textures[17], v_TexCoord * v_TilingFactor); break;
                case 18: texColor *= texture(u_Textures[18], v_TexCoord * v_TilingFactor); break;
                case 19: texColor *= texture(u_Textures[19], v_TexCoord * v_TilingFactor); break;
                case 20: texColor *= texture(u_Textures[20], v_TexCoord * v_TilingFactor); break;
                case 21: texColor *= texture(u_Textures[21], v_TexCoord * v_TilingFactor); break;
                case 22: texColor *= texture(u_Textures[22], v_TexCoord * v_TilingFactor); break;
                case 23: texColor *= texture(u_Textures[23], v_TexCoord * v_TilingFactor); break;
                case 24: texColor *= texture(u_Textures[24], v_TexCoord * v_TilingFactor); break;
                case 25: texColor *= texture(u_Textures[25], v_TexCoord * v_TilingFactor); break;
                case 26: texColor *= texture(u_Textures[26], v_TexCoord * v_TilingFactor); break;
                case 27: texColor *= texture(u_Textures[27], v_TexCoord * v_TilingFactor); break;
                case 28: texColor *= texture(u_Textures[28], v_TexCoord * v_TilingFactor); break;
                case 29: texColor *= texture(u_Textures[29], v_TexCoord * v_TilingFactor); break;
                case 30: texColor *= texture(u_Textures[30], v_TexCoord * v_TilingFactor); break;
                case 31: texColor *= texture(u_Textures[31], v_TexCoord * v_TilingFactor); break;
            }
            o_Color = texColor;
            o_EntityID = v_EntityID;
        }
    )";

    static const char* s_LineShaderSourceVertex = R"(
        #version 330 core
        layout(location = 0) in vec3 a_Position;
        layout(location = 1) in vec4 a_Color;

        uniform mat4 u_ViewProjection;

        out vec4 v_Color;

        void main()
        {
            v_Color = a_Color;
            gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
        }
    )";

    static const char* s_LineShaderSourceFragment = R"(
        #version 330 core
        layout(location = 0) out vec4 o_Color;
        layout(location = 1) out int o_EntityID;

        in vec4 v_Color;

        void main()
        {
            o_Color = v_Color;
            o_EntityID = -1;
        }
    )";

    void Renderer2D::Init()
    {
        s_Data.QuadVertexArray = VertexArray::Create();

        s_Data.QuadVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex));
        s_Data.QuadVertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float4, "a_Color" },
            { ShaderDataType::Float2, "a_TexCoord" },
            { ShaderDataType::Float,  "a_TexIndex" },
            { ShaderDataType::Float,  "a_TilingFactor" },
            { ShaderDataType::Int,    "a_EntityID" }
        });
        s_Data.QuadVertexArray->AddVertexBuffer(s_Data.QuadVertexBuffer);

        s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];

        uint32_t* quadIndices = new uint32_t[s_Data.MaxIndices];
        uint32_t offset = 0;
        for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6)
        {
            quadIndices[i + 0] = offset + 0;
            quadIndices[i + 1] = offset + 1;
            quadIndices[i + 2] = offset + 2;

            quadIndices[i + 3] = offset + 2;
            quadIndices[i + 4] = offset + 3;
            quadIndices[i + 5] = offset + 0;

            offset += 4;
        }

        Ref<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, s_Data.MaxIndices);
        s_Data.QuadVertexArray->SetIndexBuffer(quadIB);
        delete[] quadIndices;

        // 1x1 solid white texture for pure-color quads
        s_Data.WhiteTexture = Texture2D::Create(1, 1);
        uint32_t whiteTextureData = 0xffffffff;
        s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

        int32_t samplers[s_Data.MaxTextureSlots];
        for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
            samplers[i] = i;

        s_Data.TextureShader = Shader::Create("TextureShader", s_TextureShaderSourceVertex, s_TextureShaderSourceFragment);
        s_Data.TextureShader->Bind();
        s_Data.TextureShader->SetIntArray("u_Textures", samplers, s_Data.MaxTextureSlots);

        s_Data.TextureSlots[0] = s_Data.WhiteTexture;

        s_Data.QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
        s_Data.QuadVertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
        s_Data.QuadVertexPositions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
        s_Data.QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

        // ── Line batch ───────────────────────────────────────────────────────
        s_Data.LineVertexArray = VertexArray::Create();

        s_Data.LineVertexBuffer = VertexBuffer::Create(s_Data.MaxLineVertices * sizeof(LineVertex));
        s_Data.LineVertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float4, "a_Color" }
        });
        s_Data.LineVertexArray->AddVertexBuffer(s_Data.LineVertexBuffer);
        s_Data.LineVertexBufferBase = new LineVertex[s_Data.MaxLineVertices];

        s_Data.LineShader = Shader::Create("LineShader", s_LineShaderSourceVertex, s_LineShaderSourceFragment);
    }

    void Renderer2D::Shutdown()
    {
        delete[] s_Data.QuadVertexBufferBase;
        delete[] s_Data.LineVertexBufferBase;
    }

    void Renderer2D::BeginScene(const OrthographicCamera& camera)
    {
        BeginScene(camera.GetViewProjectionMatrix());
    }

    void Renderer2D::BeginScene(const glm::mat4& viewProjection)
    {
        s_Data.TextureShader->Bind();
        s_Data.TextureShader->SetMat4("u_ViewProjection", viewProjection);

        s_Data.LineShader->Bind();
        s_Data.LineShader->SetMat4("u_ViewProjection", viewProjection);

        StartBatch();
    }

    void Renderer2D::BeginScene(const glm::mat4& projection, const glm::mat4& transform)
    {
        glm::mat4 viewProj = projection * glm::inverse(transform);
        BeginScene(viewProj);
    }

    void Renderer2D::EndScene()
    {
        Flush();
        FlushLines();
    }

    void Renderer2D::StartBatch()
    {
        s_Data.QuadIndexCount = 0;
        s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
        s_Data.TextureSlotIndex = 1;

        s_Data.LineVertexCount = 0;
        s_Data.LineVertexBufferPtr = s_Data.LineVertexBufferBase;
    }

    void Renderer2D::Flush()
    {
        if (s_Data.QuadIndexCount == 0)
            return;

        uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase);
        s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);

        for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
            s_Data.TextureSlots[i]->Bind(i);

        s_Data.TextureShader->Bind();
        s_Data.QuadVertexArray->Bind();
        glDrawElements(GL_TRIANGLES, s_Data.QuadIndexCount, GL_UNSIGNED_INT, nullptr);
        s_Data.Stats.DrawCalls++;
    }

    void Renderer2D::NextBatch()
    {
        Flush();
        StartBatch();
    }

    void Renderer2D::FlushLines()
    {
        if (s_Data.LineVertexCount == 0)
            return;

        uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.LineVertexBufferPtr - (uint8_t*)s_Data.LineVertexBufferBase);
        s_Data.LineVertexBuffer->SetData(s_Data.LineVertexBufferBase, dataSize);

        s_Data.LineShader->Bind();
        s_Data.LineVertexArray->Bind();
        glLineWidth(s_Data.LineWidth);
        glDrawArrays(GL_LINES, 0, s_Data.LineVertexCount);
        s_Data.Stats.DrawCalls++;
    }

    void Renderer2D::NextLineBatch()
    {
        FlushLines();
        s_Data.LineVertexCount = 0;
        s_Data.LineVertexBufferPtr = s_Data.LineVertexBufferBase;
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, int entityID)
    {
        DrawQuad({ position.x, position.y, 0.0f }, size, color, entityID);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, int entityID)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        DrawQuad(transform, color, entityID);
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor, int entityID)
    {
        DrawQuad({ position.x, position.y, 0.0f }, size, texture, tilingFactor, tintColor, entityID);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor, int entityID)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        DrawQuad(transform, texture, tilingFactor, tintColor, entityID);
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<SubTexture2D>& subtexture, float tilingFactor, const glm::vec4& tintColor, int entityID)
    {
        DrawQuad({ position.x, position.y, 0.0f }, size, subtexture, tilingFactor, tintColor, entityID);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<SubTexture2D>& subtexture, float tilingFactor, const glm::vec4& tintColor, int entityID)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        DrawQuad(transform, subtexture, tilingFactor, tintColor, entityID);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color, int entityID)
    {
        DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, color, entityID);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color, int entityID)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        DrawQuad(transform, color, entityID);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor, int entityID)
    {
        DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, texture, tilingFactor, tintColor, entityID);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor, int entityID)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        DrawQuad(transform, texture, tilingFactor, tintColor, entityID);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<SubTexture2D>& subtexture, float tilingFactor, const glm::vec4& tintColor, int entityID)
    {
        DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, subtexture, tilingFactor, tintColor, entityID);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<SubTexture2D>& subtexture, float tilingFactor, const glm::vec4& tintColor, int entityID)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        DrawQuad(transform, subtexture, tilingFactor, tintColor, entityID);
    }

    void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID)
    {
        constexpr size_t quadVertexCount = 4;
        const float textureIndex = 0.0f; // White Texture
        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
        const float tilingFactor = 1.0f;

        if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
            NextBatch();

        for (size_t i = 0; i < quadVertexCount; i++)
        {
            s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->Color = color;
            s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
            s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
            s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
            s_Data.QuadVertexBufferPtr->EntityID = entityID;
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;
        s_Data.Stats.QuadCount++;
    }

    void Renderer2D::DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor, int entityID)
    {
        constexpr size_t quadVertexCount = 4;
        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

        if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
            NextBatch();

        float textureIndex = 0.0f;
        for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
        {
            if (*s_Data.TextureSlots[i] == *texture)
            {
                textureIndex = (float)i;
                break;
            }
        }

        if (textureIndex == 0.0f)
        {
            if (s_Data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
                NextBatch();

            textureIndex = (float)s_Data.TextureSlotIndex;
            s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
            s_Data.TextureSlotIndex++;
        }

        for (size_t i = 0; i < quadVertexCount; i++)
        {
            s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->Color = tintColor;
            s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
            s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
            s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
            s_Data.QuadVertexBufferPtr->EntityID = entityID;
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;
        s_Data.Stats.QuadCount++;
    }

    void Renderer2D::DrawQuad(const glm::mat4& transform, const Ref<SubTexture2D>& subtexture, float tilingFactor, const glm::vec4& tintColor, int entityID)
    {
        constexpr size_t quadVertexCount = 4;
        const glm::vec2* textureCoords = subtexture->GetTexCoords();
        const Ref<Texture2D> texture = subtexture->GetTexture();

        if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
            NextBatch();

        float textureIndex = 0.0f;
        for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
        {
            if (*s_Data.TextureSlots[i] == *texture)
            {
                textureIndex = (float)i;
                break;
            }
        }

        if (textureIndex == 0.0f)
        {
            if (s_Data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
                NextBatch();

            textureIndex = (float)s_Data.TextureSlotIndex;
            s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
            s_Data.TextureSlotIndex++;
        }

        for (size_t i = 0; i < quadVertexCount; i++)
        {
            s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->Color = tintColor;
            s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
            s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
            s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
            s_Data.QuadVertexBufferPtr->EntityID = entityID;
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;
        s_Data.Stats.QuadCount++;
    }

    void Renderer2D::DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color)
    {
        if (s_Data.LineVertexCount >= Renderer2DData::MaxLineVertices)
            NextLineBatch();

        s_Data.LineVertexBufferPtr->Position = p0;
        s_Data.LineVertexBufferPtr->Color = color;
        s_Data.LineVertexBufferPtr++;

        s_Data.LineVertexBufferPtr->Position = p1;
        s_Data.LineVertexBufferPtr->Color = color;
        s_Data.LineVertexBufferPtr++;

        s_Data.LineVertexCount += 2;
        s_Data.Stats.LineCount++;
    }

    void Renderer2D::DrawRect(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
    {
        glm::vec3 p0 = { position.x - size.x * 0.5f, position.y - size.y * 0.5f, position.z };
        glm::vec3 p1 = { position.x + size.x * 0.5f, position.y - size.y * 0.5f, position.z };
        glm::vec3 p2 = { position.x + size.x * 0.5f, position.y + size.y * 0.5f, position.z };
        glm::vec3 p3 = { position.x - size.x * 0.5f, position.y + size.y * 0.5f, position.z };

        DrawLine(p0, p1, color);
        DrawLine(p1, p2, color);
        DrawLine(p2, p3, color);
        DrawLine(p3, p0, color);
    }

    void Renderer2D::DrawRect(const glm::mat4& transform, const glm::vec4& color)
    {
        glm::vec3 lineVertices[4];
        for (size_t i = 0; i < 4; i++)
            lineVertices[i] = glm::vec3(transform * s_Data.QuadVertexPositions[i]);

        DrawLine(lineVertices[0], lineVertices[1], color);
        DrawLine(lineVertices[1], lineVertices[2], color);
        DrawLine(lineVertices[2], lineVertices[3], color);
        DrawLine(lineVertices[3], lineVertices[0], color);
    }

    void Renderer2D::DrawCircle(const glm::vec3& center, float radius, const glm::vec4& color, int segments)
    {
        const float twoPi = 2.0f * 3.14159265358979323846f;
        for (int i = 0; i < segments; i++)
        {
            float theta0 = ((float)i / (float)segments) * twoPi;
            float theta1 = (((float)i + 1.0f) / (float)segments) * twoPi;

            glm::vec3 p0 = { center.x + radius * cosf(theta0), center.y + radius * sinf(theta0), center.z };
            glm::vec3 p1 = { center.x + radius * cosf(theta1), center.y + radius * sinf(theta1), center.z };

            DrawLine(p0, p1, color);
        }
    }

    void Renderer2D::DrawCircle(const glm::mat4& transform, float radius, const glm::vec4& color, int segments)
    {
        const float twoPi = 2.0f * 3.14159265358979323846f;
        for (int i = 0; i < segments; i++)
        {
            float theta0 = ((float)i / (float)segments) * twoPi;
            float theta1 = (((float)i + 1.0f) / (float)segments) * twoPi;

            glm::vec4 p0Local = { radius * cosf(theta0), radius * sinf(theta0), 0.0f, 1.0f };
            glm::vec4 p1Local = { radius * cosf(theta1), radius * sinf(theta1), 0.0f, 1.0f };

            glm::vec3 p0 = glm::vec3(transform * p0Local);
            glm::vec3 p1 = glm::vec3(transform * p1Local);

            DrawLine(p0, p1, color);
        }
    }

    void Renderer2D::SetLineWidth(float width)
    {
        s_Data.LineWidth = width;
    }

    float Renderer2D::GetLineWidth()
    {
        return s_Data.LineWidth;
    }

    void Renderer2D::ResetStats()
    {
        memset(&s_Data.Stats, 0, sizeof(Statistics));
    }

    Renderer2D::Statistics Renderer2D::GetStats()
    {
        return s_Data.Stats;
    }
}
