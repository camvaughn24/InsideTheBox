// Create solid color buffer
void build_solid_color_buffer(GLuint num_vertices, vec4 color, GLuint buffer) {
    vector<vec4> obj_colors;
    for (int i = 0; i < num_vertices; i++) {
        obj_colors.push_back(color);
    }

    glBindBuffer(GL_ARRAY_BUFFER, ColorBuffers[buffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*colCoords*num_vertices, obj_colors.data(), GL_STATIC_DRAW);
}

void build_textures( ) {
    int w, h, n;
    int force_channels = 4;
    unsigned char *image_data;

    // Create textures and activate unit 0
    glGenTextures( texFiles.size(),  TextureIDs);
    glActiveTexture( GL_TEXTURE0 );

    for (int i = 0; i < texFiles.size(); i++) {
        // Load image from file
        image_data = stbi_load(texFiles[i], &w, &h, &n, force_channels);
        if (!image_data) {
            fprintf(stderr, "ERROR: could not load %s\n", texFiles[i]);
        }
        // NPOT check for power of 2 dimensions
        if ((w & (w - 1)) != 0 || (h & (h - 1)) != 0) {
            fprintf(stderr, "WARNING: texture %s is not power-of-2 dimensions\n",
                    texFiles[i]);
        }
        int width_in_bytes = w * 4;
        unsigned char *top = NULL;
        unsigned char *bottom = NULL;
        unsigned char temp = 0;
        int half_height = h / 2;

        for ( int row = 0; row < half_height; row++ ) {
            top = image_data + row * width_in_bytes;
            bottom = image_data + ( h - row - 1 ) * width_in_bytes;
            for ( int col = 0; col < width_in_bytes; col++ ) {
                temp = *top;
                *top = *bottom;
                *bottom = temp;
                top++;
                bottom++;
            }
        }

        // Bind current texture id
        glBindTexture(GL_TEXTURE_2D, TextureIDs[i]);
        // Load image data into texture
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     image_data);
        glGenerateMipmap(GL_TEXTURE_2D);
        // Set scaling modes
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        // Set wrapping modes
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // Set maximum anisotropic filtering for system
        GLfloat max_aniso = 0.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
        // set the maximum!
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);
    }
}

void load_object(GLuint obj) {
    vector<vec4> vertices;
    vector<vec2> uvCoords;
    vector<vec3> normals;

    // Load model and set number of vertices
    loadOBJ(objFiles[obj], vertices, uvCoords, normals);
    numVertices[obj] = vertices.size();

    // Create and load object buffers
    glGenBuffers(NumObjBuffers, ObjBuffers[obj]);
    glBindVertexArray(VAOs[obj]);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*posCoords*numVertices[obj], vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][NormBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*normCoords*numVertices[obj], normals.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][TexBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*texCoords*numVertices[obj], uvCoords.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

// Draw object with color
void draw_color_obj(GLuint obj, GLuint color) {
    // Select default shader program
    glUseProgram(default_program);

    // Pass projection matrix to default shader
    glUniformMatrix4fv(default_proj_mat_loc, 1, GL_FALSE, proj_matrix);

    // Pass camera matrix to default shader
    glUniformMatrix4fv(default_cam_mat_loc, 1, GL_FALSE, camera_matrix);

    // Pass model matrix to default shader
    glUniformMatrix4fv(default_model_mat_loc, 1, GL_FALSE, model_matrix);

    // Bind vertex array
    glBindVertexArray(VAOs[obj]);

    // Bind position object buffer and set attributes for default shader
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glVertexAttribPointer(default_vPos, posCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(default_vPos);

    // Bind color buffer and set attributes for default shader
    glBindBuffer(GL_ARRAY_BUFFER, ColorBuffers[color]);
    glVertexAttribPointer(default_vCol, colCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(default_vCol);

    // Draw object
    glDrawArrays(GL_TRIANGLES, 0, numVertices[obj]);
}

void draw_mat_object(GLuint obj, GLuint material){
    // Select shader program
    glUseProgram(lighting_program);

    // Pass projection and camera matrices to shader
    glUniformMatrix4fv(lighting_proj_mat_loc, 1, GL_FALSE, proj_matrix);
    glUniformMatrix4fv(lighting_camera_mat_loc, 1, GL_FALSE, camera_matrix);

    // Bind lights
    glUniformBlockBinding(lighting_program, lighting_lights_block_idx, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, LightBuffers[LightBuffer], 0, Lights.size()*sizeof(LightProperties));

    // Bind materials
    glUniformBlockBinding(lighting_program, lighting_materials_block_idx, 1);
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, MaterialBuffers[MaterialBuffer], 0, Materials.size()*sizeof(MaterialProperties));


    // Set camera position
    glUniform3fv(lighting_eye_loc, 1, eye);


    // Set num lights and lightOn
    glUniform1i(lighting_num_lights_loc, numLights);
    glUniform1iv(lighting_light_on_loc, numLights, lightOn);

    // Pass model matrix and normal matrix to shader
    glUniformMatrix4fv(lighting_model_mat_loc, 1, GL_FALSE, model_matrix);
    glUniformMatrix4fv(lighting_norm_mat_loc, 1, GL_FALSE, normal_matrix);

    // Pass material index to shader
    glUniform1i(lighting_material_loc, material);

    // Bind vertex array
    glBindVertexArray(VAOs[obj]);

    // Bind position object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glVertexAttribPointer(lighting_vPos, posCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(lighting_vPos);

    // Bind normal object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][NormBuffer]);
    glVertexAttribPointer(lighting_vNorm, normCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(lighting_vNorm);

    // Draw object
    glDrawArrays(GL_TRIANGLES, 0, numVertices[obj]);
}

void draw_tex_object(GLuint obj, GLuint texture){
    // Select shader program
    glUseProgram(texture_program);

    // Pass projection matrix to shader
    glUniformMatrix4fv(texture_proj_mat_loc, 1, GL_FALSE, proj_matrix);

    // Pass camera matrix to shader
    glUniformMatrix4fv(texture_camera_mat_loc, 1, GL_FALSE, camera_matrix);

    // Pass model matrix to shader
    glUniformMatrix4fv(texture_model_mat_loc, 1, GL_FALSE, model_matrix);

    // Bind texture
    glBindTexture(GL_TEXTURE_2D, TextureIDs[texture]);

    // Bind vertex array
    glBindVertexArray(VAOs[obj]);

    // Bind position object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glVertexAttribPointer(texture_vPos, posCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(texture_vPos);

    // Bind texture object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][TexBuffer]);
    glVertexAttribPointer(texture_vTex, texCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(texture_vTex);

    // Draw object
    glDrawArrays(GL_TRIANGLES, 0, numVertices[obj]);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);

    ww = width;
    hh = height;
}

void build_lightsaberart(){
    vector<vec4> vertices;
    vector<vec2> uvCoords;
    vector<vec3> normals;

    vertices = {
            vec4(1.0f, 3.25f, 5.5f, 1.0f),
            vec4(1.0f, 3.25f, 4.0f, 1.0f),
            vec4(1.0f, 2.25f, 4.0f, 1.0f),
            vec4(1.0f, 2.25f, 4.0f, 1.0f),
            vec4(1.0f, 2.25f, 5.5f, 1.0f),
            vec4(1.0f, 3.25f, 5.5f, 1.0f),
    };

    normals = {
            vec3(1.0f, 0.0f, 0.0f),
            vec3(1.0f, 0.0f, 0.0f),
            vec3(1.0f, 0.0f, 0.0f),
            vec3(1.0f, 0.0f, 0.0f),
            vec3(1.0f, 0.0f, 0.0f),
            vec3(1.0f, 0.0f, 0.0f),
    };

    uvCoords = {
            {1.0f, 1.0f},
            {0.0f, 1.0f},
            {0.0f, 0.0f},
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f},
    };

    // Set number of vertices
    numVertices[LightSaber] = vertices.size();

    // Create and load object buffers
    glGenBuffers(NumObjBuffers, ObjBuffers[LightSaber]);
    glBindVertexArray(VAOs[LightSaber]);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[LightSaber][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * posCoords * numVertices[LightSaberArt], vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[LightSaber][NormBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * normCoords * numVertices[LightSaber], normals.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[LightSaber][TexBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * texCoords * numVertices[LightSaber], uvCoords.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void build_vangoghart(){
    vector<vec4> vertices;
    vector<vec2> uvCoords;
    vector<vec3> normals;

    vertices = {
            vec4(1.0f, 3.25f, 2.5f, 1.0f),
            vec4(1.0f, 3.25f, 1.0f, 1.0f),
            vec4(1.0f, 2.25f, 1.0f, 1.0f),
            vec4(1.0f, 2.25f, 1.0f, 1.0f),
            vec4(1.0f, 2.25f, 2.5f, 1.0f),
            vec4(1.0f, 3.25f, 2.5f, 1.0f),
    };

    normals = {
            vec3(1.0f, 0.0f, 0.0f),
            vec3(1.0f, 0.0f, 0.0f),
            vec3(1.0f, 0.0f, 0.0f),
            vec3(1.0f, 0.0f, 0.0f),
            vec3(1.0f, 0.0f, 0.0f),
            vec3(1.0f, 0.0f, 0.0f),
    };

    uvCoords = {
            {1.0f, 1.0f},
            {0.0f, 1.0f},
            {0.0f, 0.0f},
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f},
    };

    // Set number of vertices
    numVertices[VanGogh] = vertices.size();

    // Create and load object buffers
    glGenBuffers(NumObjBuffers, ObjBuffers[VanGogh]);
    glBindVertexArray(VAOs[VanGogh]);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[VanGogh][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * posCoords * numVertices[VanGoghArt], vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[VanGogh][NormBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * normCoords * numVertices[VanGogh], normals.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[VanGogh][TexBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * texCoords * numVertices[VanGogh], uvCoords.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void build_grass() {
    vector<vec4> vertices;
    vector<vec2> uvCoords;
    vector<vec3> normals;

    vertices = {
            vec4(100.0f, 0.9f, -100.0f, 1.0f),
            vec4(-100.0f, 0.9f, -100.0f, 1.0f),
            vec4(-100.0f, 0.9f, 100.0f, 1.0f),
            vec4(-100.0f, 0.9f, 100.0f, 1.0f),
            vec4(100.0f, 0.9f, 100.0f, 1.0f),
            vec4(100.0f, 0.9f, -100.0f, 1.0f),
    };

    normals = {
            vec3(1.0f, 0.0f, 0.0f),
            vec3(1.0f, 0.0f, 0.0f),
            vec3(1.0f, 0.0f, 0.0f),
            vec3(1.0f, 0.0f, 0.0f),
            vec3(1.0f, 0.0f, 0.0f),
            vec3(1.0f, 0.0f, 0.0f),
    };


    // add grass texture coordinates
    uvCoords = {
            {50.0f, 50.0f},
            {-25.0f, 50.0f},
            {-25.0f, -25.0f},
            {-25.0f, -25.0f},
            {50.0f, -25.0f},
            {50.0f, 50.0f},
    };

    // Set number of vertices
    numVertices[Grass] = vertices.size();

    // Create and load object buffers
    glGenBuffers(NumObjBuffers, ObjBuffers[Grass]);
    glBindVertexArray(VAOs[Grass]);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[Grass][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * posCoords * numVertices[GrassTex], vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[Grass][NormBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * normCoords * numVertices[Grass], normals.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[Grass][TexBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * texCoords * numVertices[Grass], uvCoords.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}
    void build_carpet(){
        // Carpet geometry
        vector<vec4> vertices;
        vector<vec2> uvCoords;
        vector<vec3> normals;

        vertices = {
                vec4(1.125f, 1.0f, -2.25f, 1.0f),
                vec4(-1.125f, 1.0f, -2.25f, 1.0f) ,
                vec4(-1.125f, 1.0f, 5.25f, 1.0f),
                vec4(-1.125f, 1.0f, 5.25f, 1.0f),
                vec4(1.125f, 1.0f, 5.25f, 1.0f),
                vec4(1.125f, 1.0f, -2.25f, 1.0f),
        };

        normals = {
                vec3(1.0f, 0.0f, 0.0f),
                vec3(1.0f, 0.0f, 0.0f),
                vec3(1.0f, 0.0f, 0.0f),
                vec3(1.0f, 0.0f, 0.0f),
                vec3(1.0f, 0.0f, 0.0f),
                vec3(1.0f, 0.0f, 0.0f),
        };

        uvCoords = {
                {1.0f, 1.0f},
                {0.0f, 1.0f},
                {0.0f, 0.0f},
                {0.0f, 0.0f},
                {1.0f,  0.0f},
                {1.0f, 1.0f},
        };



        // Set number of vertices
    numVertices[CarpetObj] = vertices.size();

    // Create and load object buffers
    glGenBuffers(NumObjBuffers, ObjBuffers[CarpetObj]);
    glBindVertexArray(VAOs[CarpetObj]);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[CarpetObj][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * posCoords * numVertices[BlueCarpet], vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[CarpetObj][NormBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * normCoords * numVertices[CarpetObj], normals.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[CarpetObj][TexBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * texCoords * numVertices[CarpetObj], uvCoords.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}


    void build_road() {
        vector<vec4> vertices;
        vector<vec2> uvCoords;
        vector<vec3> normals;

        vertices = {
                vec4(20.0f, 1.0f, 7.5f, 1.0f),
                vec4(-20.0f, 1.0f, 7.5f, 1.0f),
                vec4(-20.0f, 1.0f, 17.5f, 1.0f),
                vec4(-20.0f, 1.0f, 17.5f, 1.0f),
                vec4(20.0f, 1.0f, 17.5f, 1.0f),
                vec4(20.0f, 1.0f, 7.5f, 1.0f),
        };

        normals = {
                vec3(1.0f, 0.0f, 0.0f),
                vec3(1.0f, 0.0f, 0.0f),
                vec3(1.0f, 0.0f, 0.0f),
                vec3(1.0f, 0.0f, 0.0f),
                vec3(1.0f, 0.0f, 0.0f),
                vec3(1.0f, 0.0f, 0.0f),
        };


        uvCoords = {
                {1.0f, 1.0f},
                {0.0f, 1.0f},
                {0.0f, 0.0f},
                {0.0f, 0.0f},
                {1.0f,  0.0f},
                {1.0f, 1.0f},
        };


        // Set number of vertices
        numVertices[Road] = vertices.size();

        // Create and load object buffers
        glGenBuffers(NumObjBuffers, ObjBuffers[Road]);
        glBindVertexArray(VAOs[Road]);
        glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[Road][PosBuffer]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * posCoords * numVertices[RoadTex], vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[Road][NormBuffer]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * normCoords * numVertices[Road], normals.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[Road][TexBuffer]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * texCoords * numVertices[Road], uvCoords.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

    }


    void build_helicopter_pad(){
        vector<vec4> vertices;
        vector<vec2> uvCoords;
        vector<vec3> normals;

        vertices = {
                vec4(-3.0f, 1.0f, 0.0f, 1.0f),
                vec4(-6.0f, 1.0f, 0.0f, 1.0f),
                vec4(-6.0f, 1.0f, 5.25f, 1.0f),
                vec4(-6.0f, 1.0f, 5.25f, 1.0f),
                vec4(-3.0f, 1.0f, 5.25f, 1.0f),
                vec4(-3.0f, 1.0f, 0.0f, 1.0f),
        };

        normals = {
                vec3(1.0f, 0.0f, 0.0f),
                vec3(1.0f, 0.0f, 0.0f),
                vec3(1.0f, 0.0f, 0.0f),
                vec3(1.0f, 0.0f, 0.0f),
                vec3(1.0f, 0.0f, 0.0f),
                vec3(1.0f, 0.0f, 0.0f),
        };

        uvCoords = {
                {1.0f, 1.0f},
                {0.0f, 1.0f},
                {0.0f, 0.0f},
                {0.0f, 0.0f},
                {1.0f,  0.0f},
                {1.0f, 1.0f},
        };



        // Set number of vertices
        numVertices[HelicopterPad] = vertices.size();

        // Create and load object buffers
        glGenBuffers(NumObjBuffers, ObjBuffers[HelicopterPad]);
        glBindVertexArray(VAOs[CarpetObj]);
        glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[HelicopterPad][PosBuffer]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * posCoords * numVertices[HelicopterPadTex], vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[HelicopterPad][NormBuffer]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * normCoords * numVertices[HelicopterPad], normals.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[HelicopterPad][TexBuffer]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * texCoords * numVertices[HelicopterPad], uvCoords.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void build_garage_door(){
        vector<vec4> vertices;
        vector<vec2> uvCoords;
        vector<vec3> normals;

        vertices = {
                vec4(5.3f, 2.95f, 4.25f, 1.0f),
                vec4(3.45f, 2.95f, 4.25f, 1.0f),
                vec4(3.45f, 1.0f, 4.25f, 1.0f),
                vec4(3.45f, 1.0f, 4.25f, 1.0f),
                vec4(5.3f, 1.0f, 4.25f, 1.0f),
                vec4(5.3f, 2.95f, 4.25f, 1.0f),
        };

        normals = {
                vec3(1.0f, 0.0f, 0.0f),
                vec3(1.0f, 0.0f, 0.0f),
                vec3(1.0f, 0.0f, 0.0f),
                vec3(1.0f, 0.0f, 0.0f),
                vec3(1.0f, 0.0f, 0.0f),
                vec3(1.0f, 0.0f, 0.0f),
        };

        uvCoords = {
                {1.0f, 1.0f},
                {0.0f, 1.0f},
                {0.0f, 0.0f},
                {0.0f, 0.0f},
                {1.0f,  0.0f},
                {1.0f, 1.0f},
        };



        // Set number of vertices
        numVertices[GarageDoor] = vertices.size();

        // Create and load object buffers
        glGenBuffers(NumObjBuffers, ObjBuffers[GarageDoor]);
        glBindVertexArray(VAOs[GarageDoor]);
        glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[GarageDoor][PosBuffer]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * posCoords * numVertices[GarageDoorTex], vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[GarageDoor][NormBuffer]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * normCoords * numVertices[GarageDoor], normals.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[GarageDoor][TexBuffer]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * texCoords * numVertices[GarageDoor], uvCoords.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void build_tv_screen(){
        vector<vec4> vertices;
        vector<vec2> uvCoords;
        vector<vec3> normals;

        vertices = {
                vec4(-1.0f, 8.0f, 10.0f, 1.0f),
                vec4(-1.0f, 8.0f, 7.0f, 1.0f),
                vec4(-1.0f, 7.0f, 7.0f, 1.0f),
                vec4(-1.0f, 7.0f, 7.0f, 1.0f),
                vec4(-1.0f, 7.0f, 10.0f, 1.0f),
                vec4(-1.0f, 8.0f, 10.0f, 1.0f),
        };

        normals = {
                vec3(1.0f, 0.0f, 0.0f),
                vec3(1.0f, 0.0f, 0.0f),
                vec3(1.0f, 0.0f, 0.0f),
                vec3(1.0f, 0.0f, 0.0f),
                vec3(1.0f, 0.0f, 0.0f),
                vec3(1.0f, 0.0f, 0.0f),
        };

        uvCoords = {
                {1.0f, 1.0f},
                {0.0f, 1.0f},
                {0.0f, 0.0f},
                {0.0f, 0.0f},
                {1.0f,  0.0f},
                {1.0f, 1.0f},
        };



        // Set number of vertices
        numVertices[TVScreen] = vertices.size();

        // Create and load object buffers
        glGenBuffers(NumObjBuffers, ObjBuffers[TVScreen]);
        glBindVertexArray(VAOs[TVScreen]);
        glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[TVScreen][PosBuffer]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * posCoords * numVertices[TVRoku], vertices.data(), GL_STATIC_DRAW);//}
        glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[TVScreen][NormBuffer]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * normCoords * numVertices[TVScreen], normals.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[TVScreen][TexBuffer]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * texCoords * numVertices[TVScreen], uvCoords.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    // draw all textures
    void draw_art(){
        glDisable(GL_CULL_FACE);
    model_matrix = translate(0.0f, -2.0f, 0.0f)*scale(4.0f, 2.0f, 2.0f);
    draw_tex_object(LightSaber, LightSaberArt);

    model_matrix = translate(0.0f, -2.0f, 2.0f)*scale(4.0f, 2.0f, 2.0f);
    draw_tex_object(VanGogh, VanGoghArt);

    draw_tex_object(Grass, GrassTex);

    draw_tex_object(Road, RoadTex);

    draw_tex_object(CarpetObj, BlueCarpet);

    draw_tex_object(HelicopterPad, HelicopterPadTex);

    draw_tex_object(GarageDoor, GarageDoorTex);

    model_matrix = translate(-3.05f, -1.0f, 18.25f) * rotate(180.0f, 0.0f, 1.0f, 0.0f)
            *scale(1.1f, 1.125f, 1.0f);


    if (tv_on)
    draw_tex_object(TVScreen, TVRoku);
        glEnable(GL_CULL_FACE);




}

    // function to stay in room
    void keep_cam_in_room(vec3 camera_eye, int projection) {
        GLfloat keep_value = .2f;
        GLfloat x_max = 3.5f;
        GLfloat x_min = -4.0f;
        GLfloat z_max = 12.3f;
        GLfloat z_min = -2.0f;
        GLfloat y_max;
        GLfloat y_min;

        // first floor
        if (projection == PERSPECTIVE_FIRST){
        y_max = 4.5f;
        y_min = 1.0f;}

        // second floor
        else {
            y_min = 6.0f;
            y_max = 10.0f;

        }

        if (camera_eye[0] >= x_max){
            eye[0] -= keep_value;
        }
        else if (camera_eye[0] <= x_min) {
            eye[0] += keep_value;
        }


        // y direction
        if (camera_eye[1] >= y_max){
           eye[1] -= keep_value;
        }
        else if (camera_eye[1] <= y_min){
            eye[1] += keep_value;
        }

        // z direction
        if ((camera_eye[2] < 2.0f) && (camera_eye[2] > -3.25) && (proj == PERSPECTIVE_SECOND)){

            z_min = 0.0f;
        }
        if (camera_eye[2] <= z_min){
            eye[2] += keep_value;
        }
        else if (camera_eye[2] >= z_max){
            eye[2] -= keep_value;
        }


    }

    // function to change floors
    GLboolean change_floors(vec3 camera_eye, int proj) {
        GLfloat x_max;
        GLfloat x_min;
        GLfloat z_max;
        GLfloat z_min;
        GLfloat y_max;
        GLfloat y_min;

        if (proj == PERSPECTIVE_FIRST) { // go upstairs
            x_min = -3.5f;
            x_max = -2.0f;
            y_max = 5.0f;
            y_min = 1.0f;
            z_max = 6.0f;
            z_min = 1.0f;

            if ((camera_eye[0] <= x_max) && (camera_eye[0] >= x_min) && (camera_eye[1] >= y_min) &&
                (camera_eye[1] <= y_max)
                && (camera_eye[2] >= z_min) && (camera_eye[2] <= z_max)) {
                eye = vec3(3.0f, 8.0f, 1.0f);
                center = vec3(3.0f, 8.0f, 10.0f);
                return true;
            }

        }

        else{ // go downstairs
            x_min = 1.0f;
            x_max = 3.5f;
            y_min = 6.0f;
            y_max = 8.0f;
            z_max = 0.0f;
            z_min = -2.0f;

            if ((camera_eye[0] <= x_max) && (camera_eye[0] >= x_min) && (camera_eye[1] >= y_min) &&
                (camera_eye[1] <= y_max)
                && (camera_eye[2] >= z_min) && (camera_eye[2] <= z_max)) {
                eye = vec3(-3.0f, 3.0f, 6.25f);
                center = vec3(-3.0f, 3.0f, 10.0f);
                return true;
            }

        }


    }

    void draw_windows(){
        // Declare transformation matrices
        model_matrix = mat4().identity();
        mat4 scale_matrix = mat4().identity();
        mat4 rot_matrix = mat4().identity();
        mat4 trans_matrix = mat4().identity();
        // draw cross windows
        // left side
        scale_matrix = scale(.25f, .5f, .6f);
        trans_matrix = translate(-4.5f, windows_posy, .25f);
        model_matrix = trans_matrix * scale_matrix;
        draw_tex_object(Window, BlackWood);

        trans_matrix = translate(-4.5f, windows_posy, 3.75f);
        model_matrix = trans_matrix * scale_matrix;
        draw_tex_object(Window, BlackWood);

        trans_matrix = translate(-4.5f, windows_posy, 6.6f);
        model_matrix = trans_matrix * scale_matrix;
        draw_tex_object(Window, BlackWood);

        trans_matrix = translate(-4.5f, windows_posy, 9.75f);
        model_matrix = trans_matrix * scale_matrix;
        draw_tex_object(Window, BlackWood);

        // right side
        trans_matrix = translate(4.5f, windows_posy, .25f);
        model_matrix = trans_matrix * scale_matrix;
        draw_tex_object(Window, BlackWood);

        trans_matrix = translate(4.5f, 2.3f, 3.65f);
        model_matrix = trans_matrix * scale_matrix;
        draw_tex_object(Window, BlackWood);

        trans_matrix = translate(4.5f, 2.3f, 6.75f);
        model_matrix = trans_matrix * scale_matrix;
        draw_tex_object(Window, BlackWood);

        trans_matrix = translate(4.5f, 2.3f, 10.0f);
        model_matrix = trans_matrix * scale_matrix;
        draw_tex_object(Window, BlackWood);

        // back

        scale_matrix = scale(.5f, .5f, .6f);
        rot_matrix = rotate(90.0f, 0.0f, 1.0f, 0.0f);
        trans_matrix = translate(-2.25f, windows_posy, -2.5f);
        model_matrix = trans_matrix * rot_matrix * scale_matrix;
        draw_tex_object(Window, BlackWood);
        trans_matrix = translate(2.35f, windows_posy, -2.5f);
        model_matrix = trans_matrix * rot_matrix * scale_matrix;
        draw_tex_object(Window, BlackWood);

}


void create_mirror( ) {
    // Clear framebuffercolo for mirror rendering pass
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // TODO: Set mirror projection matrix
    proj_matrix = frustum(-0.2f, 0.2f, -0.2f, 0.2f, 0.2f, 20.0f);

    // TODO: Set mirror camera matrix
    camera_matrix = lookat(mirror_eye, mirror_center, vec3(0.0f, -1.0f, 0.0f));

    // Render mirror scene (without mirror)
    mirror = true;
    render_scene();
    glFlush();
    mirror = false;

    // TODO: Activate texture unit 0
    glActiveTexture(GL_TEXTURE0);
    // TODO: Bind mirror texture
    glBindTexture(GL_TEXTURE_2D, TextureIDs[MirrorTex]);
    // TODO: Copy framebuffer into mirror texture
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, ww, hh, 0);

    // Reset viewport
    glViewport(0, 0, ww, hh);
}

void build_mirror( ) {
    // Generate mirror texture
    glGenTextures(1, &TextureIDs[MirrorTex]);
    // Bind mirror texture
    glBindTexture(GL_TEXTURE_2D, TextureIDs[MirrorTex]);
    // TODO: Create empty mirror texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ww, hh, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}


// Debug mirror renderer
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad(GLuint shader, GLuint tex)
{
    // reset viewport
    glViewport(0, 0, ww, hh);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // render Depth map to quad for visual debugging
    // ---------------------------------------------
    glUseProgram(shader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TextureIDs[tex]);
    if (quadVAO == 0)
    {
        float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}












