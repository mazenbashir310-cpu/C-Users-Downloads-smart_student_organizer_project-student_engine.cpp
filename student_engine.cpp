// ============================================================================
// منظّم حياة الطالب الذكي — محرك العمليات الحسابية والخوارزميات المتقدمة (C++17)
// Smart Student Organizer — Core Algorithmic & Scheduling Engine
// ============================================================================
// يتولى هذا المحرك العمليات المعقدة في المشروع:
// 1. حساب الأوزان الديناميكية ومؤشر الأولوية (Priority Score) بدقة الساعات والأيام.
// 2. رصد المهام العاجلة التي يتبقى عليها أقل من 24 ساعة لإرسال إشعارات النظام.
// 3. خوارزمية الجدولة الذكية (14 يوماً) مع مراعاة أوقات المحاضرات وساعات الفراغ.
// 4. التحليل الإحصائي للأداء الأكاديمي وتوقع المعدل الفصلي (GPA & Weighted Grade).
// ============================================================================

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <map>

// ----------------------------------------------------------------------------
// هياكل البيانات الأساسية (Data Structures)
// ----------------------------------------------------------------------------

struct Task {
    int id;
    std::string title;
    std::string subject;
    std::string type;        // "اختبار", "مشروع", "واجب", "مراجعة"
    std::string dueDate;     // YYYY-MM-DD HH:MM
    double hoursLeft;        // الساعات المتبقية حتى الموعد النهائي
    double estHours;         // الساعات التقديرية لإنجاز المهمة
    int difficulty;          // من 1 إلى 5
    bool completed;
    double priorityScore;    // يُحسب عبر محرك C++
    std::string urgencyTag;  // "متأخر", "أقل من 24 ساعة", "عاجل جداً", "قريب", "مجدول"
};

struct Lecture {
    std::string day;
    std::string subject;
    std::string startTime;   // HH:MM
    std::string endTime;     // HH:MM
    double durationHours;
};

struct GradeRecord {
    std::string subject;
    std::string assessment;
    double score;
    double maxScore;
    double weight;           // الوزن النسبي للتقييم
};

struct StudySession {
    int dayOffset;
    std::string dateStr;
    std::string arabicDay;
    std::string subject;
    std::string taskTitle;
    std::string taskType;
    double allocatedHours;
    double priorityScore;
};

// ----------------------------------------------------------------------------
// فئة المحرك الذكي (SmartStudentEngine Class)
// ----------------------------------------------------------------------------

class SmartStudentEngine {
private:
    double dailyFreeHours;
    std::vector<Task> tasks;
    std::vector<Lecture> lectures;
    std::vector<GradeRecord> grades;

    // تحديد وزن نوع المهمة
    double getTypeWeight(const std::string& type) const {
        if (type == "اختبار") return 3.0;
        if (type == "مشروع")  return 2.5;
        if (type == "واجب")   return 1.5;
        return 1.2; // مراجعة أو قراءة
    }

public:
    explicit SmartStudentEngine(double freeHours = 4.5)
        : dailyFreeHours(freeHours) {}

    void addTask(int id, const std::string& title, const std::string& subject,
                 const std::string& type, const std::string& dueDate,
                 double hoursLeft, double estHours, int difficulty, bool completed = false) {
        Task t{id, title, subject, type, dueDate, hoursLeft, estHours, difficulty, completed, 0.0, ""};
        tasks.push_back(t);
    }

    void addLecture(const std::string& day, const std::string& subject,
                    const std::string& startTime, const std::string& endTime, double durationHours) {
        lectures.push_back({day, subject, startTime, endTime, durationHours});
    }

    void addGrade(const std::string& subject, const std::string& assessment,
                  double score, double maxScore, double weight = 1.0) {
        if (maxScore <= 0) maxScore = 1.0;
        if (score > maxScore) score = maxScore; // منع تجاوز الدرجة العظمى
        grades.push_back({subject, assessment, score, maxScore, weight});
    }

    // ------------------------------------------------------------------------
    // 1. خوارزمية حساب الأولوية وتصنيف حالة الطوارئ (بما فيها تنبيه < 24 ساعة)
    // ------------------------------------------------------------------------
    std::vector<Task> computePrioritizedTasks() {
        std::vector<Task> active;
        for (const auto& task : tasks) {
            if (task.completed) continue;

            Task enriched = task;
            double weight = getTypeWeight(enriched.type);
            double daysLeft = std::ceil(enriched.hoursLeft / 24.0);

            // معادلة الاستعجال المطورة: تعطي وزناً مضاعفاً للمهام التي يتبقى عليها أقل من 24 ساعة
            double urgencyScore = 0.0;
            if (enriched.hoursLeft <= 0) {
                urgencyScore = 120.0;
                enriched.urgencyTag = "🚨 متأخر / مستحق الآن";
            } else if (enriched.hoursLeft <= 24.0) {
                urgencyScore = 100.0 + (24.0 - enriched.hoursLeft) * 1.5;
                enriched.urgencyTag = "🔔 عاجل: أقل من 24 ساعة!";
            } else if (daysLeft <= 3) {
                urgencyScore = std::round((55.0 / daysLeft) * 10.0) / 10.0;
                enriched.urgencyTag = "⚠️ عاجل جداً (خلال 3 أيام)";
            } else if (daysLeft <= 7) {
                urgencyScore = std::round((50.0 / daysLeft) * 10.0) / 10.0;
                enriched.urgencyTag = "⏳ قريب (خلال أسبوع)";
            } else {
                urgencyScore = std::round((50.0 / daysLeft) * 10.0) / 10.0;
                enriched.urgencyTag = "📅 مجدول";
            }

            // إدخال كثافة العمل (الساعات التقديرية) مع الصعوبة ووزن النوع
            double workloadFactor = std::min(enriched.estHours * 1.5, 18.0);
            enriched.priorityScore = std::round(((weight * 10.0) + (enriched.difficulty * 5.0) + urgencyScore + workloadFactor) * 10.0) / 10.0;

            active.push_back(enriched);
        }

        // ترتيب تنازلي حسب نقاط الأولوية، ثم تصاعدي حسب الساعات المتبقية
        std::sort(active.begin(), active.end(), [](const Task& a, const Task& b) {
            if (std::abs(a.priorityScore - b.priorityScore) > 0.05) {
                return a.priorityScore > b.priorityScore;
            }
            return a.hoursLeft < b.hoursLeft;
        });

        return active;
    }

    // ------------------------------------------------------------------------
    // 2. استخراج المهام التي تتطلب إشعاراً فورياً (أقل من 24 ساعة)
    // ------------------------------------------------------------------------
    std::vector<Task> getUrgent24HourNotifications() {
        std::vector<Task> prioritized = computePrioritizedTasks();
        std::vector<Task> alerts;
        for (const auto& t : prioritized) {
            if (t.hoursLeft > 0.0 && t.hoursLeft <= 24.0) {
                alerts.push_back(t);
            }
        }
        return alerts;
    }

    // ------------------------------------------------------------------------
    // 3. خوارزمية توليد خطة المذاكرة الذكية لمدة 14 يوماً
    // ------------------------------------------------------------------------
    std::vector<StudySession> generateSmartStudyPlan(const std::vector<std::string>& next14Dates,
                                                     const std::vector<std::string>& next14ArabicDays) {
        std::vector<Task> queue = computePrioritizedTasks();
        std::vector<StudySession> plan;

        // حساب ساعات المحاضرات لكل يوم لتعديل وقت الفراغ الفعلي
        std::map<std::string, double> lectureLoadByDay;
        for (const auto& lec : lectures) {
            lectureLoadByDay[lec.day] += lec.durationHours;
        }

        int horizon = std::min({14, static_cast<int>(next14Dates.size()), static_cast<int>(next14ArabicDays.size())});

        for (int dayOffset = 0; dayOffset < horizon; ++dayOffset) {
            std::string currentDayName = next14ArabicDays[dayOffset];
            // تقليص طفيف في ساعات المذاكرة إذا كان اليوم مزدحماً بالمحاضرات (> 4 ساعات محاضرات)
            double dayLectureHours = lectureLoadByDay[currentDayName];
            double penalty = (dayLectureHours > 4.0) ? std::min(1.5, (dayLectureHours - 4.0) * 0.5) : 0.0;
            double availableHours = std::max(1.0, dailyFreeHours - penalty);

            for (auto& task : queue) {
                if (task.estHours <= 0.05) continue;

                int maxAllowedDay = std::max(0, static_cast<int>(std::ceil(task.hoursLeft / 24.0)));
                if (dayOffset <= maxAllowedDay && availableHours > 0.05) {
                    // الحد الأقصى للجلسة الواحدة للمادة في اليوم 2.5 ساعة لمنع الإرهاق الذهني
                    double maxSession = (task.hoursLeft <= 24.0) ? 3.5 : 2.5;
                    double allocated = std::min({availableHours, task.estHours, maxSession});
                    allocated = std::round(allocated * 2.0) / 2.0; // تقريب لأقرب نصف ساعة (0.5)

                    if (allocated <= 0.0 && task.estHours > 0.0) {
                        allocated = std::min(availableHours, task.estHours);
                    }

                    if (allocated > 0.0) {
                        task.estHours -= allocated;
                        availableHours -= allocated;
                        plan.push_back({
                            dayOffset,
                            next14Dates[dayOffset],
                            currentDayName,
                            task.subject,
                            task.title,
                            task.type,
                            std::round(allocated * 10.0) / 10.0,
                            task.priorityScore
                        });
                    }
                }
            }
        }
        return plan;
    }

    // ------------------------------------------------------------------------
    // 4. حساب متوسطات المواد والمعدل العام الموزون (Academic Analytics)
    // ------------------------------------------------------------------------
    std::map<std::string, double> calculateSubjectAverages() const {
        std::map<std::string, double> weightedSum;
        std::map<std::string, double> totalWeight;

        for (const auto& g : grades) {
            double pct = (g.score / g.maxScore) * 100.0;
            weightedSum[g.subject] += pct * g.weight;
            totalWeight[g.subject] += g.weight;
        }

        std::map<std::string, double> averages;
        for (const auto& pair : weightedSum) {
            averages[pair.first] = std::round((pair.second / totalWeight[pair.first]) * 10.0) / 10.0;
        }
        return averages;
    }

    double calculateOverallGPA5() const {
        auto subjAvgs = calculateSubjectAverages();
        if (subjAvgs.empty()) return 0.0;
        double sumPts = 0.0;
        for (const auto& pair : subjAvgs) {
            double p = pair.second;
            double gp = 1.0;
            if (p >= 95.0) gp = 5.00;
            else if (p >= 90.0) gp = 4.75;
            else if (p >= 85.0) gp = 4.50;
            else if (p >= 80.0) gp = 4.00;
            else if (p >= 75.0) gp = 3.50;
            else if (p >= 70.0) gp = 3.00;
            else if (p >= 65.0) gp = 2.50;
            else if (p >= 60.0) gp = 2.00;
            sumPts += gp;
        }
        return std::round((sumPts / subjAvgs.size()) * 100.0) / 100.0;
    }
};

// ----------------------------------------------------------------------------
// نقطة التشغيل الرئيسية لاختبار المحرك وإخراج النتائج بصيغة JSON القياسية
// ----------------------------------------------------------------------------
int main() {
    SmartStudentEngine engine(4.5);

    // إضافة مهام تجريبية (تتضمن مهمة عاجلة يتبقى عليها 16 ساعة لاختبار نظام تنبيه الـ 24 ساعة)
    engine.addTask(1, "الاختبار النصفي", "الإحصاء والاحتمالات", "اختبار", "2026-10-20 09:00", 16.0, 6.0, 5, false);
    engine.addTask(2, "تصميم قاعدة بيانات", "قواعد البيانات SQL", "مشروع", "2026-10-24 23:59", 120.0, 5.0, 4, false);
    engine.addTask(3, "تحليل البيانات بـ Pandas", "برمجة بايثون", "واجب", "2026-10-26 23:59", 168.0, 3.0, 3, false);
    engine.addTask(4, "اختبار قصير (Quiz 2)", "الذكاء الاصطناعي", "اختبار", "2026-10-29 11:00", 234.0, 4.0, 4, false);

    // إضافة محاضرات
    engine.addLecture("الأحد", "برمجة بايثون", "08:00", "10:00", 2.0);
    engine.addLecture("الإثنين", "قواعد البيانات SQL", "10:00", "12:00", 2.0);
    engine.addLecture("الثلاثاء", "الإحصاء والاحتمالات", "09:00", "11:00", 2.0);
    engine.addLecture("الأربعاء", "الذكاء الاصطناعي", "11:00", "13:00", 2.0);

    // إضافة درجات
    engine.addGrade("برمجة بايثون", "الاختبار الأول", 19.0, 20.0, 1.0);
    engine.addGrade("قواعد البيانات SQL", "الاختبار الأول", 17.5, 20.0, 1.0);
    engine.addGrade("الإحصاء والاحتمالات", "الاختبار الأول", 15.0, 20.0, 1.0);
    engine.addGrade("الذكاء الاصطناعي", "واجب عملي", 9.0, 10.0, 1.0);

    auto prioritized = engine.computePrioritizedTasks();
    auto alerts24h   = engine.getUrgent24HourNotifications();
    auto subjAvgs    = engine.calculateSubjectAverages();
    double gpa5      = engine.calculateOverallGPA5();

    std::vector<std::string> dates = {
        "2026-10-19", "2026-10-20", "2026-10-21", "2026-10-22",
        "2026-10-23", "2026-10-24", "2026-10-25"
    };
    std::vector<std::string> arabicDays = {
        "الإثنين", "الثلاثاء", "الأربعاء", "الخميس", "الجمعة", "السبت", "الأحد"
    };
    auto studyPlan = engine.generateSmartStudyPlan(dates, arabicDays);

    std::cout << "{\n";
    std::cout << "  \"engine\": \"SmartStudentEngine C++17\",\n";
    std::cout << "  \"estimated_gpa_out_of_5\": " << std::fixed << std::setprecision(2) << gpa5 << ",\n";
    std::cout << "  \"urgent_24h_notifications_count\": " << alerts24h.size() << ",\n";
    std::cout << "  \"prioritized_tasks\": [\n";
    for (size_t i = 0; i < prioritized.size(); ++i) {
        const auto& t = prioritized[i];
        std::cout << "    {\"id\": " << t.id
                  << ", \"title\": \"" << t.title << "\""
                  << ", \"subject\": \"" << t.subject << "\""
                  << ", \"hours_left\": " << std::setprecision(1) << t.hoursLeft
                  << ", \"priority_score\": " << t.priorityScore
                  << ", \"status\": \"" << t.urgencyTag << "\"}"
                  << (i + 1 < prioritized.size() ? "," : "") << "\n";
    }
    std::cout << "  ],\n";
    std::cout << "  \"study_plan_sessions\": " << studyPlan.size() << "\n";
    std::cout << "}\n";

    return 0;
}
