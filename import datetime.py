import streamlit as st
import streamlit.components.v1 as components
import pathlib

st.set_page_config(
    page_title="منظّم حياة الطالب الذكي",
    page_icon="🎓",
    layout="wide",
    initial_sidebar_state="collapsed"
)

# إخفاء الهوامش الافتراضية في Streamlit ليظهر تطبيق HTML/JS بملء الشاشة
st.markdown("""
<style>
    .block-container { padding: 0 !important; max-width: 100% !important; }
    header[data-testid="stHeader"] { display: none !important; }
    footer { display: none !important; }
</style>
""", unsafe_allow_html=True)

html_path = pathlib.Path(__file__).parent / "smart_student_organizer.html"
if html_path.exists():
    html_content = html_path.read_text(encoding="utf-8")
    components.html(html_content, height=1150, scrolling=True)
else:
    st.error("لم يتم العثور على ملف smart_student_organizer.html في نفس المجلد!")
