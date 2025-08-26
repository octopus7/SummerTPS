bl_info = {
    "name": "Separate Loose Islands (per Object)",
    "author": "ChatGPT",
    "version": (1, 1, 1),
    "blender": (3, 0, 0),
    "location": "Object menu / Context menu / F3",
    "description": "한 메시의 분리된 파츠를 개별 오브젝트로 분리. 원본명 유지, 새 파츠만 {원본명}_{번호} 부여",
    "category": "Object",
}

import bpy
from mathutils import Vector

def apply_all_modifiers(obj, depsgraph):
    obj_eval = obj.evaluated_get(depsgraph)
    new_mesh = bpy.data.meshes.new_from_object(
        obj_eval, preserve_all_data_layers=True
    )
    old_mesh = obj.data
    obj.modifiers.clear()
    obj.data = new_mesh
    if old_mesh and old_mesh.users == 0:
        bpy.data.meshes.remove(old_mesh)

def world_bbox_center(obj):
    corners = [obj.matrix_world @ Vector(c) for c in obj.bound_box]
    if not corners:
        return obj.matrix_world.to_translation()
    acc = Vector((0.0, 0.0, 0.0))
    for c in corners:
        acc += c
    return acc / len(corners)

class VIEW3D_OT_separate_loose_islands(bpy.types.Operator):
    bl_idname = "object.separate_loose_islands_per_object"
    bl_label = "Separate Loose Islands (per Object)"
    bl_options = {'REGISTER', 'UNDO'}

    only_selected : bpy.props.BoolProperty(
        name="선택한 오브젝트만", default=True,
        description="선택한 오브젝트만 처리 (끄면 씬의 모든 메시 오브젝트 처리)"
    )
    apply_mods : bpy.props.BoolProperty(
        name="모디파이어 적용 후 분리", default=False,
        description="모디파이어 결과를 굳힌 뒤 분리 (파괴적)"
    )
    apply_rot_scale : bpy.props.BoolProperty(
        name="회전/스케일 적용", default=False,
        description="분리 전에 Rotation/Scale 적용"
    )
    origin_to_geometry : bpy.props.BoolProperty(
        name="오리진을 지오메트리로", default=True,
        description="분리된 오브젝트들의 오리진을 지오메트리 중심(바운딩)으로 이동"
    )
    rename_parts : bpy.props.BoolProperty(
        name="새 파츠만 이름 재지정", default=True,
        description="원본 오브젝트 이름은 그대로 두고, 새 파츠에만 번호를 붙임"
    )
    name_pattern : bpy.props.StringProperty(
        name="이름 패턴", default="{base}_{index:02d}",
        description="새 파츠 이름 규칙. {base}=원본명, {index}=1부터 시작"
    )
    order_by : bpy.props.EnumProperty(
        name="번호 순서 기준",
        items=[
            ('X', "X (왼→오른)", "바운딩 중심 X 오름차순"),
            ('Y', "Y (앞→뒤)", "바운딩 중심 Y 오름차순"),
            ('Z', "Z (아래→위)", "바운딩 중심 Z 오름차순"),
            ('NAME', "이름", "이름 사전순"),
            ('NONE', "정렬 안 함", "블렌더 생성 순서"),
        ],
        default='X'
    )

    def invoke(self, context, event):
        return self.execute(context)

    def execute(self, context):
        view_layer = context.view_layer
        depsgraph = context.evaluated_depsgraph_get()

        targets = ([o for o in context.selected_objects if o.type == 'MESH']
                   if self.only_selected else
                   [o for o in context.scene.objects if o.type == 'MESH'])

        if not targets:
            self.report({'WARNING'}, "처리할 메시 오브젝트가 없습니다.")
            return {'CANCELLED'}

        orig_active = view_layer.objects.active
        orig_selection = context.selected_objects[:]

        try:
            for obj in targets:
                bpy.ops.object.mode_set(mode='OBJECT')
                bpy.ops.object.select_all(action='DESELECT')
                obj.select_set(True)
                view_layer.objects.active = obj

                if self.apply_mods:
                    apply_all_modifiers(obj, depsgraph)
                if self.apply_rot_scale:
                    bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)

                # Edit 모드에서 Loose 분리
                bpy.ops.object.mode_set(mode='EDIT')
                bpy.ops.mesh.reveal()
                bpy.ops.mesh.select_mode(type="VERT")
                bpy.ops.mesh.select_all(action='SELECT')
                bpy.ops.mesh.separate(type='LOOSE')
                bpy.ops.object.mode_set(mode='OBJECT')

                # 분리로 생성된 모든 파츠(원본 포함)가 선택됨
                parts = [o for o in context.selected_objects if o.type == 'MESH']
                if len(parts) <= 1:
                    bpy.ops.object.select_all(action='DESELECT')
                    continue

                # 원본 유지, 새 파츠만 추출
                new_parts = [p for p in parts if p != obj]

                # 번호 부여 순서
                if self.order_by == 'NAME':
                    new_parts.sort(key=lambda o: o.name)
                elif self.order_by == 'X':
                    new_parts.sort(key=lambda o: world_bbox_center(o).x)
                elif self.order_by == 'Y':
                    new_parts.sort(key=lambda o: world_bbox_center(o).y)
                elif self.order_by == 'Z':
                    new_parts.sort(key=lambda o: world_bbox_center(o).z)
                # NONE: 정렬하지 않음

                # 새 파츠 이름 붙이기
                if self.rename_parts:
                    base = obj.name
                    for idx, part in enumerate(new_parts, start=1):
                        try:
                            part.name = self.name_pattern.format(base=base, index=idx)
                        except Exception:
                            part.name = f"{base}_{idx:02d}"

                # 오리진 정리
                if self.origin_to_geometry:
                    bpy.ops.object.origin_set(type='ORIGIN_GEOMETRY', center='BOUNDS')

                bpy.ops.object.select_all(action='DESELECT')

        finally:
            bpy.ops.object.mode_set(mode='OBJECT')
            bpy.ops.object.select_all(action='DESELECT')
            for o in orig_selection:
                if o and o.name in context.scene.objects:
                    o.select_set(True)
            if orig_active and orig_active.name in context.scene.objects:
                view_layer.objects.active = orig_active

        self.report({'INFO'}, f"처리 완료: {len(targets)}개 오브젝트")
        return {'FINISHED'}

# ----- Menus (Blender 4.x 호환: 존재하는 메뉴에만 추가) -----

def menu_func_object(self, context):
    self.layout.separator()
    self.layout.operator(
        VIEW3D_OT_separate_loose_islands.bl_idname,
        icon='MOD_EXPLODE',
        text="Separate Loose Islands (per Object)"
    )

def register():
    bpy.utils.register_class(VIEW3D_OT_separate_loose_islands)

    # Object 상단 메뉴
    if hasattr(bpy.types, "VIEW3D_MT_object"):
        bpy.types.VIEW3D_MT_object.append(menu_func_object)

    # Object 우클릭 컨텍스트 메뉴
    if hasattr(bpy.types, "VIEW3D_MT_object_context_menu"):
        bpy.types.VIEW3D_MT_object_context_menu.append(menu_func_object)

def unregister():
    # 제거도 가드
    if hasattr(bpy.types, "VIEW3D_MT_object"):
        try: bpy.types.VIEW3D_MT_object.remove(menu_func_object)
        except Exception: pass
    if hasattr(bpy.types, "VIEW3D_MT_object_context_menu"):
        try: bpy.types.VIEW3D_MT_object_context_menu.remove(menu_func_object)
        except Exception: pass

    bpy.utils.unregister_class(VIEW3D_OT_separate_loose_islands)

if __name__ == "__main__":
    register()
